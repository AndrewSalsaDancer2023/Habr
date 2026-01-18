#include "stackcontext.h"

stack_context fixedsize_stack::allocate(std::size_t size)
{
    void * vp = std::malloc(size);
    if (!vp) {
        throw std::bad_alloc();
    }

        // stack_context sctx;
        // sctx.size = size_;
        // sctx.sp = static_cast< char * >( vp) + sctx.size;

        // return sctx;
        // return stack_context(size_, static_cast< char * >(vp) + size_);
    return stack_context(size, static_cast< char * >(vp) + size);
}

void fixedsize_stack::deallocate( stack_context & sctx)
{
    void * vp = static_cast< char * >( sctx.sp) - sctx.size;
    std::free(vp);
}

stack_context protected_stack::allocate(std::size_t size)
{
        long page_size = get_page_size();
        if(page_size == -1)
            throw std::logic_error("impossible to get page size");
        // calculate how many pages are required
        // const std::size_t pages = (size_ + page_size - 1) / page_size;
        const std::size_t pages = (size + page_size - 1) / page_size;
        // add one page at bottom that will be used as guard-page
        const std::size_t size__ = (pages + 1) * page_size;

        void * vp = ::mmap(0, size__, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);

        if (MAP_FAILED == vp) throw std::bad_alloc();

        // conforming to POSIX.1-2001
        const int result = ::mprotect(vp, page_size, PROT_NONE);
        if(result == -1)
            throw std::system_error(errno, std::generic_category());
        // stack_context sctx;
        // sctx.size = size__;
        // sctx.sp = static_cast< char * >( vp) + sctx.size;

        // return sctx;
        // return stack_context(size_, static_cast< char * >(vp) + size_);
        return stack_context(size, static_cast< char * >(vp) + size);
}

void protected_stack::deallocate(stack_context& sctx)
{
    if(!sctx.sp)
        throw std::logic_error("sctx.sp schoudn't be null");

    void * vp = static_cast< char * >( sctx.sp) - sctx.size;
    // conform to POSIX.4 (POSIX.1b-1993, _POSIX_C_SOURCE=199309L)
    ::munmap( vp, sctx.size);
}
