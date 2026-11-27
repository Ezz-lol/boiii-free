// Standalone synthetic-context test. No exception is raised and no game is run.
#include <vector>
#include "../src/common/exception/minidump.hpp"
#include "../src/common/exception/fault_memory.hpp"

#include <cassert>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>

int main(int argc, char** argv)
{
    SYSTEM_INFO system{};
    GetSystemInfo(&system);
    const auto size = system.dwPageSize;
    assert(size >= 4096 && size <= exception::fault_memory::max_page_size);
    auto* const allocation = static_cast<unsigned char*>(VirtualAlloc(nullptr, 5ull * size,
        MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
    assert(allocation);
    struct release_allocation
    {
        void* value;
        ~release_allocation() { VirtualFree(value, 0, MEM_RELEASE); }
    } release{allocation};
    for (unsigned int page = 1; page <= 3; ++page)
        for (unsigned int offset = 0; offset < size; ++offset)
            allocation[page * size + offset] = static_cast<unsigned char>((offset * 37u + page * 53u) & 255u);
    DWORD old{};
    assert(VirtualProtect(allocation, size, PAGE_NOACCESS, &old));
    assert(VirtualProtect(allocation + size, 3ull * size, PAGE_EXECUTE_READ, &old));
    assert(VirtualProtect(allocation + 4ull * size, size, PAGE_NOACCESS, &old));

    CONTEXT context{};
    RtlCaptureContext(&context);
    context.Rip = reinterpret_cast<std::uintptr_t>(allocation + 2ull * size + 0x123);
    EXCEPTION_RECORD record{};
    record.ExceptionCode = EXCEPTION_ACCESS_VIOLATION;
    record.ExceptionAddress = reinterpret_cast<void*>(context.Rip);
    record.NumberParameters = 2;
    record.ExceptionInformation[0] = 0;
    record.ExceptionInformation[1] = 1;
    EXCEPTION_POINTERS pointers{&record, &context};
    const auto selected = exception::fault_memory::snapshot(&pointers);
    assert(selected.count == 3 && selected.pages[0].base == reinterpret_cast<std::uintptr_t>(allocation + 2ull * size));

    // Call the actual family writer twice to catch cursor state escaping a write.
    for (int attempt = 0; attempt < 2; ++attempt)
    {
        auto data = exception::create_minidump(&pointers);
        assert(!data.empty());
        PMINIDUMP_DIRECTORY directory{};
        void* raw{};
        ULONG bytes{};
        assert(MiniDumpReadDumpStream(data.data(), MemoryListStream, &directory, &raw, &bytes));
        const auto* list = static_cast<const MINIDUMP_MEMORY_LIST*>(raw);
        assert(bytes >= offsetof(MINIDUMP_MEMORY_LIST, MemoryRanges));
        assert(list->NumberOfMemoryRanges <= (bytes - offsetof(MINIDUMP_MEMORY_LIST, MemoryRanges)) / sizeof(MINIDUMP_MEMORY_DESCRIPTOR));
        for (ULONG i = 0; i < list->NumberOfMemoryRanges; ++i)
        {
            const auto& range = list->MemoryRanges[i];
            assert(range.StartOfMemoryRange <= UINT64_MAX - range.Memory.DataSize);
            const auto end = range.StartOfMemoryRange + range.Memory.DataSize;
            const auto allocation_base = reinterpret_cast<std::uintptr_t>(allocation);
            if (range.StartOfMemoryRange < allocation_base + 5ull * size && end > allocation_base)
            {
                assert(range.StartOfMemoryRange >= allocation_base + size);
                assert(end <= allocation_base + 4ull * size);
            }
        }
        unsigned int found{};
        for (unsigned int page = 1; page <= 3; ++page)
        {
            const auto address = reinterpret_cast<std::uintptr_t>(allocation + static_cast<std::size_t>(page) * size);
            for (ULONG i = 0; i < list->NumberOfMemoryRanges; ++i)
            {
                const auto& range = list->MemoryRanges[i];
                if (address < range.StartOfMemoryRange) continue;
                const auto offset = address - range.StartOfMemoryRange;
                if (offset > range.Memory.DataSize || size > range.Memory.DataSize - offset) continue;
                assert(range.Memory.Rva <= data.size() && offset <= data.size() - range.Memory.Rva);
                const auto at = range.Memory.Rva + offset;
                assert(size <= data.size() - at);
                assert(std::memcmp(data.data() + at, allocation + static_cast<std::size_t>(page) * size, size) == 0);
                ++found; break;
            }
        }
        assert(found == 3);
        assert(MiniDumpReadDumpStream(data.data(), ModuleListStream, &directory, &raw, &bytes));
        const auto* modules = static_cast<const MINIDUMP_MODULE_LIST*>(raw);
        assert(bytes >= offsetof(MINIDUMP_MODULE_LIST, Modules));
        assert(modules->NumberOfModules <= (bytes - offsetof(MINIDUMP_MODULE_LIST, Modules)) / sizeof(MINIDUMP_MODULE));
        for (ULONG i = 0; i < modules->NumberOfModules; ++i)
        {
            const auto& module = modules->Modules[i];
            const auto fault = context.Rip;
            assert(fault < module.BaseOfImage || fault - module.BaseOfImage >= module.SizeOfImage);
        }
        if (argc == 2)
        {
            const std::string path = std::string(argv[1]) + "-" + std::to_string(attempt + 1) + ".private.dmp";
            std::ofstream output(path, std::ios::binary);
            output.write(data.data(), static_cast<std::streamsize>(data.size()));
            assert(output.good());
        }
        std::printf("PASS: synthetic context dump %d contains exact three private executable pages (%lu bytes each), absent from module list; writer returned %zu bytes; no crash/game/runtime reproduction\n",
                    attempt + 1, size, data.size());
    }
}
