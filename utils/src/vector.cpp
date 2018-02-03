

#include <stdlib.h> // malloc, free, NULL 
#include <string.h> // memset, memcpy 

#include "utils/vector.hpp"


// Default number of entries allocated must be a power of 2 minus 1 
#define VECTOR_DEFAULT_ENTRY 15u


//
// @brief Checks vector state
//
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


//
// @brief Update the value of the tail
//
static inline void vector_update_tail(struct vector *vec)
{
        uint16_t i;

        if (vector_check(vec) == false)
        {
                return;
        }

        // Look for a non-empty element 
        i = vec->tail - 1;
        while (i != UINT16_MAX)
        {
                char *begin = (char *) vec->array;

                if (memcmp(begin + i * vec->size, vec->empty, vec->size) == 0)
                {
                        i--;
                        continue;
                }

                vec->tail = i + 1;

                return;
        }

        // Vector is empty 
        vec->tail = 0;
        return;
}


//
// @brief Reserve space for a specified number of elements
//
// @param elem_max : Maximum number of elements desired
//
// @return true on success, false on failure
//
static inline bool vector_realloc(struct vector *vec, uint32_t elem_max)
{
        char *array;

        //
        // Refuse to realloc if:
        //   - vector is inconsistent
        //   - too much entries asked
        //   - tail is beyond last new element
        //
        if ((vector_check(vec) == false)   ||
            (elem_max  > UINT16_MAX) ||
            (vec->tail > elem_max))
        {
                return false;
        }

        if (elem_max == vec->max)
        {
                // Nothing to do 
                return true;
        }

        array = (char *) realloc(vec->array, (elem_max) * vec->size);
        if (array == NULL)
        {
                return false;
        }

        if (vec->max < elem_max)
        {
                // Fill the new area with empty value 
                memset(array + vec->max * vec->size, *((char *) vec->empty), (elem_max - vec->max) * vec->size);
        }

        vec->max   = elem_max;
        vec->array = array;

        return true;
}


//
// @brief Creates a vector
//
// @param size  : Size of each element of the vector
// @param empty : Value that defines empty element
//
struct vector * vector_create(size_t size, char empty)
{
        struct vector *vec;

        if (size == 0)
        {
                return NULL;
        }

        vec = (struct vector *) malloc(sizeof(*vec));
        if (vec == NULL)
        {
                return NULL;
        }

        // Allocate the empty element 
        vec->empty = malloc(size);
        if (vec->empty == NULL)
        {
                free(vec);
                return NULL;
        }

        // Allocate by default VECTOR_DEFAULT_ENTRY entries 
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


//
// @brief Delete a vector
//
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


//
// @brief Insert an element at a given index
//
// @return true on success, false on failure
//
bool vector_insert(struct vector *vec, void *elem, uint16_t i)
{
        char *begin;

        if (vector_check(vec) == false || (i == UINT16_MAX))
        {
                //
                // Vector inconsistent or index too high (we lose
                // 1 element because of the tail and nb value being on 16 bits)
                //
                return false;
        }

        if (i >= vec->max)
        {
                uint32_t p;

                // Round up to a power of two minus 1 and realloc 
                p = (vec->max + 1) << 1;
                while (p < i)
                {
                        p <<= 1;
                }
                p--;

                if (vector_realloc(vec, p) == false)
                {
                        return false;
                }
        }

        begin = (char *) vec->array;
        memcpy(begin + i * vec->size, elem, vec->size);
        vec->nb++;

        if (i >= vec->tail)
        {
                vec->tail = i + 1;
        }

        return true;
}


//
// @brief Remove an element at a given index
//
void vector_remove(struct vector *vec, uint16_t i)
{
        char *begin;

        if ((vector_check(vec) == false) || (i >= vec->max))
        {
                return;
        }

        begin = (char *) vec->array;
        memset(begin + i * vec->size, *((char *) vec->empty), vec->size);
        vec->nb--;

        if ((i + 1) == vec->tail)
        {
                vector_update_tail(vec);
        }
}


//
// @brief Find a given element in the vector
//
// @return Index of the element on success, -1 on failure
//
uint16_t vector_find(struct vector *vec, void* elem)
{
        uint16_t i;

        if (vector_check(vec) == false)
        {
                return -1;
        }

        for (i = 0; i < vec->max; i++)
        {
                char *begin;

                begin = (char *) vec->array;
                if (memcmp(begin + i * vec->size, elem, vec->size) != 0)
                {
                        continue;
                }

                // Found 
                return i;
        }

        // Not found 
        return -1;
}


