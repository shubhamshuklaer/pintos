#include "thread_heap.h"

/*
compare function should return true if first argument needs to moves up in the heap..!!
*/


static void thread_swap(struct thread **heap,int i,int j){
	struct thread *temp;
	temp=heap[i];
	heap[i]=heap[j];
	heap[j]=temp;
} 


bool update_pos(struct thread ** heap,int location,int heap_size,thread_compare comp){
	if(heap==NULL||heap_size==0||comp==NULL||location>=heap_size||location<0)
		return false;
	int left_child,right_child,parent;
	bool updated=false,percolate_down=false;
	parent=(location-1)/2;
	if(location==0)
		percolate_down=true;
	else if(!comp(heap[location],heap[parent]))
		percolate_down=true;

	if(percolate_down){
		do{
			left_child=2*location+1;
			right_child=2*location+2;
			if(right_child>=heap_size){
				if(left_child>=heap_size){// no child exist
					updated=true;
				}else{//left child exist
					if(comp(heap[left_child],heap[location])){
						thread_swap(heap,left_child,location);
						location=left_child;
					}
					else{
						updated=true;// no need for swaping further down as the rest is already heap
					}
				}
			}else{//both childs exist
				if(comp(heap[left_child],heap[right_child])){
					if(comp(heap[left_child],heap[location])){
						thread_swap(heap,left_child,location);
						location=left_child;
					}
					else{
						updated=true;
					}
				}else{
					if(comp(heap[right_child],heap[location])){
						thread_swap(heap,right_child,location);
						location=right_child;
					}
					else{
						updated=true;
					}
				}
			}
		}while(!updated);
	}else{//percolate up
		do{
			parent=(location-1)/2;
			if(location==0){
				updated=true;
			}else{
				if(comp(heap[location],heap[parent])){
					thread_swap(heap,location,parent);
					location=parent;
				}else{
					updated=true;
				}
			}
		}while(!updated);
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



bool heapify(struct thread ** heap,int location,int heap_size,thread_compare comp){
	if(heap==NULL||heap_size==0||comp==NULL||location>=heap_size||location<0)
		return false;

	int left_child,right_child;
	bool updated=false;
	do{
		left_child=2*location+1;
		right_child=2*location+2;
		if(right_child>=heap_size){
			if(left_child>=heap_size){// no child exist
				updated=true;
			}else{//left child exist
				if(comp(heap[left_child],heap[location])){
					thread_swap(heap,left_child,location);
					location=left_child;
				}
				else{
					updated=true;// no need for swaping further down as the rest is already heap
				}
			}
		}else{//both childs exist
			if(comp(heap[left_child],heap[right_child])){
				if(comp(heap[left_child],heap[location])){
					thread_swap(heap,left_child,location);
					location=left_child;
				}
				else{
					updated=true;
				}
			}else{
				if(comp(heap[right_child],heap[location])){
					thread_swap(heap,right_child,location);
					location=right_child;
				}
				else{
					updated=true;
				}
			}
		}
	}while(!updated);
	return true;

}

bool build_heap(struct thread ** heap,int heap_size,thread_compare comp){
	int i;
	for(i=heap_size/2-1;i>=0;i--){
		heapify(heap, i, heap_size, comp);		
	}
}