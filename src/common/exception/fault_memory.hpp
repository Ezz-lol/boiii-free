#pragma once

#include <Windows.h>
#include <DbgHelp.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace exception::fault_memory
{
    // Additional memory only: the existing minidump flags remain unchanged.
    // One fault page, then at most its two immediate executable neighbours.
    constexpr std::size_t max_pages = 3;
    constexpr std::uint32_t max_page_size = 64 * 1024;

    struct region
    {
        std::uint64_t base{};
        std::uint64_t size{};
        std::uint32_t state{};
        std::uint32_t protection{};
    };

    struct page
    {
        std::uint64_t base{};
        std::uint32_t size{};
    };

    struct plan
    {
        std::array<page, max_pages> pages{};
        std::size_t count{};
        std::size_t cursor{};
    };

    inline bool readable_executable(const std::uint32_t protection) noexcept
    {
        if ((protection & (PAGE_GUARD | PAGE_NOACCESS)) != 0) return false;
        const auto access = protection & 0xFFu;
        return access == PAGE_EXECUTE_READ || access == PAGE_EXECUTE_READWRITE
            || access == PAGE_EXECUTE_WRITECOPY;
    }

    inline bool contains_page(const region& value, const std::uint64_t base,
                              const std::uint32_t size) noexcept
    {
        if (value.state != MEM_COMMIT || !readable_executable(value.protection)
            || base < value.base) return false;
        const auto offset = base - value.base;
        return offset <= value.size && size <= value.size - offset;
    }

    // The injected query makes the complete selection policy testable without
    // reading process memory. There are at most three queries, before DbgHelp.
    template <typename Query>
    plan prepare(const std::uint64_t fault, const std::uint32_t page_size, Query&& query) noexcept
    {
        plan result{};
        if (!fault || page_size < 4096 || page_size > max_page_size
            || (page_size & (page_size - 1)) != 0) return result;
        constexpr auto address_limit = static_cast<std::uint64_t>((std::numeric_limits<std::uintptr_t>::max)());
        const auto centre = fault - fault % page_size;
        if (!centre || centre > address_limit - page_size) return result;
        const auto append = [&](const std::uint64_t base)
        {
            region value{};
            if (!base || base > address_limit - page_size || !query(base, value)
                || !contains_page(value, base, page_size)) return false;
            result.pages[result.count++] = {base, page_size};
            return true;
        };
        // Do not collect unrelated neighbours when the fault itself is not an
        // eligible executable page (e.g. an invalid instruction pointer).
        if (!append(centre)) return result;
        if (centre >= page_size) append(centre - page_size);
        if (centre <= address_limit - page_size) append(centre + page_size);
        return result;
    }

    inline plan snapshot(const EXCEPTION_POINTERS* const exceptioninfo) noexcept
    {
        std::uint64_t fault{};
        if (exceptioninfo && exceptioninfo->ExceptionRecord)
            fault = reinterpret_cast<std::uintptr_t>(exceptioninfo->ExceptionRecord->ExceptionAddress);
        if (!fault && exceptioninfo && exceptioninfo->ContextRecord)
            fault = exceptioninfo->ContextRecord->Rip;
        SYSTEM_INFO system{};
        GetSystemInfo(&system);
        return prepare(fault, system.dwPageSize, [](const std::uint64_t address, region& output) noexcept
        {
            MEMORY_BASIC_INFORMATION value{};
            if (VirtualQuery(reinterpret_cast<const void*>(static_cast<std::uintptr_t>(address)),
                             &value, sizeof(value)) != sizeof(value)) return false;
            output = {reinterpret_cast<std::uintptr_t>(value.BaseAddress), value.RegionSize,
                      value.State, value.Protect};
            return true;
        });
    }

    inline BOOL CALLBACK callback(PVOID context, const PMINIDUMP_CALLBACK_INPUT input,
                                  PMINIDUMP_CALLBACK_OUTPUT output) noexcept
    {
        if (!input || !output) return FALSE;
        switch (input->CallbackType)
        {
        case MemoryCallback:
        {
            auto* const selected = static_cast<plan*>(context);
            output->MemoryBase = 0;
            output->MemorySize = 0;
            if (!selected || selected->count > max_pages || selected->cursor >= selected->count
                || selected->cursor >= max_pages) return FALSE;
            const auto& value = selected->pages[selected->cursor];
            constexpr auto address_limit = static_cast<std::uint64_t>((std::numeric_limits<std::uintptr_t>::max)());
            if (!value.base || value.size < 4096 || value.size > max_page_size
                || (value.size & (value.size - 1)) != 0 || value.base % value.size != 0
                || value.base > address_limit - value.size) return FALSE;
            ++selected->cursor;
            output->MemoryBase = value.base;
            output->MemorySize = value.size;
            return TRUE;
        }
        case RemoveMemoryCallback:
            output->MemoryBase = 0;
            output->MemorySize = 0;
            return FALSE; // No default regions are excluded.
        case CancelCallback:
            output->CheckCancel = FALSE;
            output->Cancel = FALSE;
            return TRUE;
        case WriteKernelMinidumpCallback:
            return FALSE; // Preserve the existing user-mode-only writer.
        case IncludeVmRegionCallback:
            output->Continue = TRUE;
            return TRUE;
        case IsProcessSnapshotCallback:
        case VmStartCallback:
        case IoStartCallback:
            output->Status = S_OK; // Real process, no alternate VM or I/O provider.
            return TRUE;
        case ReadMemoryFailureCallback:
            output->Status = S_OK; // Existing MiniDumpIgnoreInaccessibleMemory policy.
            return TRUE;
        default:
            // Keep SDK-prepared module/thread/secondary write flags untouched.
            return TRUE;
        }
    }

    static_assert(std::is_convertible_v<decltype(&callback), MINIDUMP_CALLBACK_ROUTINE>);
}
