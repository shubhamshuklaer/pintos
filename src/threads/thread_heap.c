#include "thread_heap.h"

/*
Max heap
*/


bool update_pos(struct thread ** heap,int location,int heap_size,thread_compare comp){
	if(heap==NULL||heap_size==0||location>=heap_size||comp==NULL){
		return false;
	}
	int left_child,right_child,parent;
	bool updated=false,percolate_down=false;
	struct thread *temp;
	if(heap_size==1)
		return true;
	if(heap_size-1==location){
		do{	
			if(location>0){
				parent=(location-1)/2;
				if( comp(heap[location],heap[parent]) ){
					temp=heap[parent];
					heap[parent]=heap[location];
					heap[location]=temp;
					location=parent;
				}else{
					updated=true;
				}
			}else{
				updated=true;
			}
		}while(!updated);
	}else{
		parent=(location-1)/2;
		if(location==0)
			percolate_down=true;
		else if( comp(heap[parent],heap[location]) )
			percolate_down=true;

		//percolate down
		if(percolate_down){
			do{
				left_child=2*location+1;
				right_child=2*location+2;
				if(right_child>=heap_size){
					if(left_child>=heap_size){
						//leaf node cannot percolate down
						updated=true;
					}else{
						if( comp(heap[left_child],heap[location]) ){
							temp=heap[location];
							heap[location]=heap[left_child];
							heap[left_child]=temp;
							location=left_child;
						}else{
							updated=true;
						}
					}
				}else{// both child will be under heap size
					if( comp(heap[left_child],heap[right_child]) ){
						if( comp(heap[left_child],heap[location]) ){
							temp=heap[location];
							heap[location]=heap[left_child];
							heap[left_child]=temp;
							location=left_child;
						}else{
							updated=true;
						}
					}else{
						if( comp(heap[right_child],heap[location]) ){
							temp=heap[location];
							heap[location]=heap[right_child];
							heap[right_child]=temp;
							location=right_child;
						}else{
							updated=true;
						}
					}
				}
			}while(!updated);
		}else{ //percolate up
			do{
				if(location>0){
					parent=(location-1)/2;
					if( comp(heap[location],heap[parent]) ){
						temp=heap[parent];
						heap[parent]=heap[location];
						heap[location]=temp;
						location=parent;
					}else{
						updated=true;
					}
				}else{
					updated=true;
				}
			}while(!updated);
		}
	}
	return true;

}



/*
heap_size needs to be decremented after calling function 
*/
struct thread * pop_top(struct thread ** heap,int heap_size,thread_compare comp){
	if(heap==NULL||heap_size==0||comp==NULL){
		return NULL;
	}
	struct thread *temp;
	temp=heap[0];
	heap[0]=heap[heap_size-1];
	heap[heap_size-1]=temp;
	update_pos(heap,0,heap_size-1,comp);
	return heap[heap_size-1];
}


/*
heap_size needs to be incremented after calling insert
*/
bool insert(struct thread ** heap,struct thread * element,int heap_size,thread_compare comp){
	if(heap==NULL||element==NULL||comp==NULL){
		return false;
	}
	heap[heap_size]=element;//place the element at last
	return update_pos(heap,heap_size,heap_size+1,comp);
}


bool heapify(struct thread ** heap,int heap_size,thread_compare comp){
	int i;
	for(i=heap_size-1;i>=0;i--)
		update_pos(heap,i,heap_size,comp);
	return true;
}