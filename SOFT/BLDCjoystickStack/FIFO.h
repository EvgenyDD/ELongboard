/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef FIFO_QUEUE_H
#define FIFO_QUEUE_H


#ifdef __cplusplus
extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include <stdint.h>


/* Exported types ------------------------------------------------------------*/
typedef struct{
	uint32_t pushPtr;
	uint32_t popPtr;
	void* ptrArray;
	uint32_t size;
}FifoQueueType;


/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/

/* Create static array */
#define FIFO_create(containerType, fifoName, size)	\
				containerType fifoName##_container[size]; \
				FifoQueueType fifoName = {0, 0, (containerType*)&fifoName##_container[0], size};\


/* Push element to FIFO queue */
#define FIFO_push(containerType, element, fifoName, action) \
				do{ \
					if(++fifoName.pushPtr >= fifoName.size) \
					{ \
						fifoName.pushPtr = 0; \
					} \
					if(fifoName.pushPtr != fifoName.popPtr) /* prevent elements override */ \
					{ \
						((containerType*)fifoName.ptrArray)[fifoName.pushPtr] = element; \
					} \
					else \
					{ /* roll back */ \
						fifoName.pushPtr = (fifoName.pushPtr == 0)?(fifoName.size - 1):(fifoName.pushPtr-1); \
						(action); \
					} \
				} \
				while(0);


/* Pop element from FIFO queue */
#define FIFO_pop(containerType, element, fifoName) \
				do{ \
					if(++fifoName.popPtr >= fifoName.size) \
					{ \
						fifoName.popPtr = 0; \
					} \
					element = ((containerType*)fifoName.ptrArray)[fifoName.popPtr];\
				} \
				while(0);

/* Get the queue size (occupied) */
#define FIFO_size(fifoName) ((fifoName.pushPtr >= fifoName.popPtr)? \
								(fifoName.pushPtr - fifoName.popPtr): \
								(fifoName.size + fifoName.pushPtr - fifoName.popPtr))


/* Exported define -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


#ifdef __cplusplus
};
#endif


#endif //FIFO_QUEUE_H
