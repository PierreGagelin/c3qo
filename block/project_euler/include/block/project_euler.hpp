#ifndef BLOCK_PROJECT_EULER_HPP
#define BLOCK_PROJECT_EULER_HPP

// Project headers
#include "c3qo/block.hpp"

struct project_euler : block
{
    explicit project_euler(struct manager *mgr);
    virtual ~project_euler() override final;

    virtual void conf_(char *conf) override final;
};

#endif // BLOCK_PROJECT_EULER_HPP
