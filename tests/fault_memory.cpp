#include "../src/common/exception/fault_memory.hpp"

#include <cassert>
#include <cstdio>

using namespace exception::fault_memory;

int main()
{
    constexpr std::uint32_t size = 4096;
    std::size_t calls{};
    const auto committed = [&](const std::uint64_t address, region& value)
    {
        ++calls;
        value = {address, size, MEM_COMMIT, PAGE_EXECUTE_READ};
        return true;
    };
    auto all = prepare(0x12345, size, committed);
    assert(all.count == 3 && calls == 3);
    assert(all.pages[0].base == 0x12000 && all.pages[1].base == 0x11000 && all.pages[2].base == 0x13000);
    for (const auto& page : all.pages) assert(page.size == size);

    for (const auto protection : {PAGE_EXECUTE_READ, PAGE_EXECUTE_READWRITE, PAGE_EXECUTE_WRITECOPY})
        assert(readable_executable(protection));
    for (const auto protection : {PAGE_EXECUTE, PAGE_NOACCESS, PAGE_READONLY, PAGE_READWRITE,
                                  PAGE_WRITECOPY, PAGE_EXECUTE_READ | PAGE_GUARD,
                                  PAGE_EXECUTE_READWRITE | PAGE_GUARD, 0})
        assert(!readable_executable(protection));
    assert(readable_executable(PAGE_EXECUTE_READ | PAGE_TARGETS_INVALID));

    for (const auto state : {MEM_FREE, MEM_RESERVE})
        assert(!contains_page({0x10000, 0x10000, static_cast<std::uint32_t>(state), PAGE_EXECUTE_READ}, 0x12000, size));
    assert(contains_page({0x10000, 0x10000, MEM_COMMIT, PAGE_EXECUTE_READ}, 0x12000, size));
    assert(!contains_page({0x12001, size, MEM_COMMIT, PAGE_EXECUTE_READ}, 0x12000, size));
    assert(!contains_page({0x12000, size - 1, MEM_COMMIT, PAGE_EXECUTE_READ}, 0x12000, size));
    assert(!contains_page({0x11000, 0x1FFF, MEM_COMMIT, PAGE_EXECUTE_READ}, 0x12000, size));

    for (const auto invalid_size : {0u, 1u, 2048u, 4095u, 4097u, 65537u, 131072u})
    {
        calls = 0;
        assert(prepare(0x12345, invalid_size, committed).count == 0 && calls == 0);
    }
    calls = 0;
    assert(prepare(0, size, committed).count == 0 && calls == 0);
    assert(prepare(UINT64_MAX, size, committed).count == 0 && calls == 0);
    auto large = prepare(0x234567, 65536, [](const auto address, region& value)
    {
        value = {address, 65536, MEM_COMMIT, PAGE_EXECUTE_READ}; return true;
    });
    assert(large.count == 3 && large.pages[0].size == 65536);

    calls = 0;
    const auto missing_fault = prepare(0x12345, size, [&](const auto, region&)
    {
        ++calls; return false;
    });
    assert(missing_fault.count == 0 && calls == 1);
    const auto gap = prepare(0x12345, size, [](const auto address, region& value)
    {
        if (address == 0x11000) return false;
        value = {address, size, MEM_COMMIT, PAGE_EXECUTE_READ}; return true;
    });
    assert(gap.count == 2 && gap.pages[0].base == 0x12000 && gap.pages[1].base == 0x13000);
    const auto protected_neighbour = prepare(0x12345, size, [](const auto address, region& value)
    {
        value = {address, size, MEM_COMMIT,
                 static_cast<std::uint32_t>(address == 0x13000 ? PAGE_READWRITE : PAGE_EXECUTE_READ)};
        return true;
    });
    assert(protected_neighbour.count == 2);

    MINIDUMP_CALLBACK_INPUT input{};
    MINIDUMP_CALLBACK_OUTPUT output{};
    input.CallbackType = MemoryCallback;
    auto independent = all;
    for (std::size_t i = 0; i < 3; ++i)
    {
        assert(callback(&all, &input, &output));
        assert(output.MemoryBase == all.pages[i].base && output.MemorySize == size);
    }
    for (int i = 0; i < 4; ++i)
    {
        output.MemoryBase = UINT64_MAX; output.MemorySize = UINT32_MAX;
        assert(!callback(&all, &input, &output));
        assert(output.MemoryBase == 0 && output.MemorySize == 0);
    }
    assert(callback(&independent, &input, &output) && output.MemoryBase == 0x12000);
    auto corrupt = independent; corrupt.count = 4;
    assert(!callback(&corrupt, &input, &output));
    corrupt = independent; corrupt.cursor = 500;
    assert(!callback(&corrupt, &input, &output));
    corrupt = independent; corrupt.cursor = 0; corrupt.pages[0].size = UINT32_MAX;
    assert(!callback(&corrupt, &input, &output) && output.MemorySize == 0);
    corrupt.pages[0] = {0x12001, size};
    assert(!callback(&corrupt, &input, &output));
    assert(!callback(nullptr, &input, &output));
    assert(!callback(&all, nullptr, &output));
    assert(!callback(&all, &input, nullptr));
    for (const auto kind : {ModuleCallback, ThreadCallback, ThreadExCallback, IncludeModuleCallback,
                            IncludeThreadCallback, SecondaryFlagsCallback})
    {
        input.CallbackType = kind; output.ModuleWriteFlags = 0x12345678;
        assert(callback(&all, &input, &output) && output.ModuleWriteFlags == 0x12345678);
    }
    input.CallbackType = CancelCallback; output.CheckCancel = TRUE; output.Cancel = TRUE;
    assert(callback(&all, &input, &output) && !output.CheckCancel && !output.Cancel);
    input.CallbackType = RemoveMemoryCallback;
    assert(!callback(&all, &input, &output) && output.MemoryBase == 0 && output.MemorySize == 0);
    input.CallbackType = WriteKernelMinidumpCallback;
    assert(!callback(&all, &input, &output));
    for (const auto kind : {IoStartCallback, VmStartCallback, IsProcessSnapshotCallback, ReadMemoryFailureCallback})
    {
        input.CallbackType = kind; output.Status = E_FAIL;
        assert(callback(&all, &input, &output) && output.Status == S_OK);
    }
    input.CallbackType = IncludeVmRegionCallback; output.Continue = FALSE;
    assert(callback(&all, &input, &output) && output.Continue);
    std::puts("PASS: bounded fault-page selection, executable/readable protection, gaps, overflow, callback exhaustion/defaults and independent write contexts");
}
