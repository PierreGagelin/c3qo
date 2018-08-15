#ifndef BLOCK_PROJECT_EULER_HPP
#define BLOCK_PROJECT_EULER_HPP

// Project headers
#include "c3qo/block.hpp"

struct bk_project_euler : block
{
    bk_project_euler(struct manager *mgr);
    virtual ~bk_project_euler() override final;

    virtual void conf_(char *conf) override final;
};

#endif // BLOCK_PROJECT_EULER_HPP
