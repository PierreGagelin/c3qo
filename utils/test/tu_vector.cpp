/**
 * @brief Test file for the vector library
 */


#include <vector>
#include <queue>

#include <stdlib.h> /* NULL */

#include "utils/vector.hpp" /* vector */

#include "gtest/gtest.h"


/* Structure to be held by the vector */
struct elem
{
        int a;
        int b;
};


class tu_vector : public testing::Test
{
        void SetUp();
        void TearDown();
};

void tu_vector::SetUp()
{
}

void tu_vector::TearDown()
{
}


TEST_F(tu_vector, append_remove)
{
        struct vector *v;
        struct elem    e1;
        struct elem    e2;

        /* Prepare 2 elements */
        e1.a = 1;
        e1.b = 1;
        e1.a = 2;
        e1.b = 2;

        v = vector_create(sizeof(struct elem), 0);
        ASSERT_TRUE(v != NULL);
        ASSERT_TRUE(v->size == sizeof(struct elem));

        /* Add two elements */
        ASSERT_TRUE(vector_append(v, &e1) == true);
        ASSERT_TRUE(v->tail == 1u);
        ASSERT_TRUE(v->nb   == 1u);

        ASSERT_TRUE(vector_append(v, &e2) == true);
        ASSERT_TRUE(v->tail == 2u);
        ASSERT_TRUE(v->nb   == 2u);

        /* Remove last element */
        vector_remove(v, 1);
        ASSERT_TRUE(v->tail == 1u);
        ASSERT_TRUE(v->nb   == 1u);

        /* Add one element */
        ASSERT_TRUE(vector_append(v, &e2) == true);
        ASSERT_TRUE(v->tail == 2u);
        ASSERT_TRUE(v->nb   == 2u);

        /* Remove first element */
        vector_remove(v, 0);
        ASSERT_TRUE(v->tail == 2u);
        ASSERT_TRUE(v->nb   == 1u);

        vector_delete(v);
}


TEST_F(tu_vector, append)
{
        struct vector *v;
        struct elem    e;

        e.a = 1;
        e.b = 1;
        v = vector_create(sizeof(struct elem), 0);
        ASSERT_TRUE(v != NULL);
        ASSERT_TRUE(v->size == sizeof(struct elem));

        for (unsigned int i = 0; i < 4096; i++)
        {
                ASSERT_TRUE(vector_append(v, &e) == true);
                ASSERT_TRUE(v->tail == i + 1);
                ASSERT_TRUE(v->nb   == i + 1);
        }

        vector_delete(v);
}


TEST_F(tu_vector, std_queue)
{
        std::queue<struct elem> v = std::queue<struct elem>();
        struct elem             e;

        e.a = 1;
        e.b = 1;

        for (unsigned int i = 0; i < 4096; i++)
        {
                v.push(e);
        }

        while (v.empty() == false)
        {
                v.pop();
        }
}


TEST_F(tu_vector, std_vector)
{
        std::vector<struct elem> v;
        struct elem              e;

        e.a = 1;
        e.b = 1;

        for (unsigned int i = 0; i < 4096; i++)
        {
                v.push_back(e);
        }
}


