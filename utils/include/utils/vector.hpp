

#ifndef UTILS_VECTOR_H
#define UTILS_VECTOR_H


#include <stdbool.h> // bool 
#include <stdint.h>  // uint16_t  


//
// @struct vector
//
struct vector
{
        void     *array; // Contiguous array of elements 
        void     *empty; // Empty element 
        uint16_t  tail;  // Index of the last element 
        uint16_t  max;   // Maximum number of element 
        uint16_t  nb;    // Number of stored elements 
        size_t    size;  // Size of an element 
};


// Create and delete the vector structure 
struct vector * vector_create(size_t elem_size, char empty);
void            vector_delete(struct vector *vec);


// Manage elements of the vector array 
bool     vector_insert(struct vector *vec, void *elem, uint16_t i);
uint16_t vector_find  (struct vector *vec, void *elem);
void     vector_remove(struct vector *vec, uint16_t i);


//
// @brief Add an element at the end of the vector
//
inline bool vector_append(struct vector *vec, void *elem)
{
        return vector_insert(vec, elem, vec->tail);
}


#endif


