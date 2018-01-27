

#include <stdlib.h> /* malloc, free, NULL */
#include <string.h> /* memset, memcpy */

#include "utils/vector.h"


#define VECTOR_DEFAULT_ENTRY 1 << 3


/**
 * @brief Checks vector state
 */
static inline bool vector_check(struct vector *vec)
{
        bool ok;

        ok = true;

        ok = ok && (vec != NULL);

        ok = ok && (vec->array     != NULL);
        ok = ok && (vec->max  != 0);
        ok = ok && (vec->size != 0);

        ok = ok && (vec->nb   <= vec->max);
        ok = ok && (vec->tail <= vec->max);

        ok = ok && (vec->nb <= vec->tail);

        return ok;
}


/**
 * @brief Update the value of the tail
 */
static inline void vector_update_tail(struct vector *vec)
{
        uint16_t i;

        if (vector_check(vec) == false)
        {
                return;
        }

        /* Look for a non-empty element */
        i = vec->tail - 1;
        while (i != UINT16_MAX)
        {
                if (memcmp(vec->array + i * vec->size, vec->empty, vec->size) == 0)
                {
                        i--;
                        continue;
                }

                vec->tail = i + 1;

                return;
        }

        /* Vector is empty */
        vec->tail = 0;
        return;
}


/**
 * @brief Double the size of the vector
 *
 * @return true on success, false on failure
 */
static inline bool vector_realloc(struct vector *vec)
{
        void *array;

        if ((vector_check(vec) == false) || (vec->max >= (UINT16_MAX >> 1)))
        {
                return false;
        }

        /* Double the maximum number of elements */
        array = realloc(vec->array, (vec->max << 1) * vec->size);
        if (array == NULL)
        {
                return false;
        }

        /* Fill the new area with empty value */
        memset(array + vec->max * vec->size, *((char *) vec->empty), vec->max * vec->size);

        vec->max <<= 1;
        vec->array = array;

        return true;
}


/**
 * @brief Creates a vector
 *
 * @param size  : Size of each element of the vector
 * @param empty : Value that defines empty element
 */
struct vector * vector_create(size_t size, char empty)
{
        struct vector *vec;

        if (size == 0)
        {
                return NULL;
        }

        vec = malloc(sizeof(*vec));
        if (vec == NULL)
        {
                return NULL;
        }

        /* Allocate the empty element */
        vec->empty = malloc(size);
        if (vec->empty == NULL)
        {
                free(vec);
                return NULL;
        }

        /* Allocate by default VECTOR_DEFAULT_ENTRY entries */
        vec->array = malloc(VECTOR_DEFAULT_ENTRY * size);
        if (vec->array == NULL)
        {
                free(vec);
                free(vec->empty);
                return NULL;
        }

        memset(vec->array, empty, VECTOR_DEFAULT_ENTRY * size);
        memset(vec->empty, empty, size);
        vec->tail = 0;
        vec->max  = VECTOR_DEFAULT_ENTRY;
        vec->nb   = 0;
        vec->size = size;

        return vec;
}


/**
 * @brief Delete a vector
 */
void vector_delete(struct vector *vec)
{
        if (vec == NULL)
        {
                return;
        }

        if (vec->array != NULL)
        {
                free(vec->array);
        }

        if (vec->empty != NULL)
        {
                free(vec->empty);
        }

        free(vec);
}


/**
 * @brief Add an element at the end of the vector
 */
bool vector_append(struct vector *vec, void *elem)
{
        if ((vector_check(vec) == false) || (elem == NULL))
        {
                return false;
        }

        /* Increase the size of the vector if necessary */
        if (vec->nb == vec->max)
        {
                /* No more room in vector */
                if (vector_realloc(vec) == false)
                {
                        return false;
                }
        }

        memcpy(vec->array + vec->tail * vec->size, elem, vec->size);
        vec->nb++;
        vec->tail++;

        return true;
}


/**
 * @brief Insert an element at a given index
 *
 * @return true on success, false on failure
 */
bool vector_insert(struct vector *vec, void *elem, uint16_t i)
{
        if ((vector_check(vec) == false) || (i >= vec->max))
        {
                return false;
        }

        memcpy(vec->array + i * vec->size, elem, vec->size);
        vec->nb++;

        if (i >= vec->tail)
        {
                vec->tail = i + 1;
        }

        return true;
}


/**
 * @brief Remove an element at a given index
 */
void vector_remove(struct vector *vec, uint16_t i)
{
        if ((vector_check(vec) == false) || (i >= vec->max))
        {
                return;
        }

        memset(vec->array + i * vec->size, 0, vec->size);
        vec->nb--;

        if ((i + 1) == vec->tail)
        {
                vector_update_tail(vec);
        }
}


/**
 * @brief Find a given element in the vector
 *
 * @return Index of the element on success, -1 on failure
 */
uint16_t vector_find(struct vector *vec, void* elem)
{
        uint16_t i;

        if (vector_check(vec) == false)
        {
                return -1;
        }

        for (i = 0; i < vec->max; i++)
        {
                if (memcmp(vec->array + i * vec->size, elem, vec->size) != 0)
                {
                        continue;
                }

                /* Found */
                return i;
        }

        /* Not found */
        return -1;
}


