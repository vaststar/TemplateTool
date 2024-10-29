#pragma once

#ifndef RETURN_FROM_LAMBDA_IF_DEAD
#define RETURN_FROM_LAMBDA_IF_DEAD(x) \
    auto sharedThis = x.lock();       \
    if (!sharedThis)                  \
        return;
#endif