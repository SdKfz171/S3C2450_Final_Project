/*****************************************************************************

*                                                                            *

*  -------------------------------- list.c --------------------------------  *

*                                                                            *

*****************************************************************************/


#include <stdio.h>
#include <stdlib.h>

#include <string.h>



#include "list.h"



/*****************************************************************************

*                                                                            *

*  ------------------------------- list_init ------------------------------  *

*                                                                            *

*****************************************************************************/



void list_init(List *list, void (*destroy)(void *data)) {

list->size = 0;

list->destroy = (void *)destroy;

list->head = NULL;

list->tail = NULL;

return;

}



void list_destroy(List *list) {



void  *data;




while (list_size(list) > 0) {



   if (list_rem_next(list, NULL, (void **)&data) == 0 && list->destroy !=

      NULL) {



      list->destroy(data);



   }



}




memset(list, 0, sizeof(List));



return;



}



int list_ins_next(List *list, ListElmt *element, char *data) {



ListElmt           *new_element;




if ((new_element = (ListElmt *)malloc(sizeof(ListElmt))) == NULL)

   return -1;


strcpy(new_element->data,data);


if (element == NULL) {


   if (list_size(list) == 0)

      list->tail = new_element;



   new_element->next = list->head;

   list->head = new_element;

   }

else {


   if (element->next == NULL)

      list->tail = new_element;



   new_element->next = element->next;

   element->next = new_element;


}

list->size++;

return 0;

}




int list_rem_next(List *list, ListElmt *element, void **data) {



ListElmt           *old_element;


if (list_size(list) == 0)

   return -1;

if (element == NULL) {

   *data = list->head->data;

   old_element = list->head;

   list->head = list->head->next;



   if (list_size(list) == 1)
      list->tail = NULL;
   }



else {


   if (element->next == NULL)

      return -1;



   *data = element->next->data;

   old_element = element->next;

   element->next = element->next->next;



   if (element->next == NULL)

      list->tail = element;



}

free(old_element);

list->size--;

return 0;

}

static void print_list(const List *list)
{
	int i ;
	ListElmt *pt = list -> head ; 
	char save[20];
	for(i = 0; i <list_size(list); i++){
		
		printf("%s\n", ((char *)(pt->data) ));
		pt = pt->next;
	}

	
}


//int add_element(List *list,ListElmt *element,void **data)
//{
//
//	
//	ListElmt           *new_element;
//		
//
//
//	if ((new_element = (ListElmt *)malloc(sizeof(ListElmt))) == NULL)
//
//    return -1;
//
//	if(element == NULL) return -1;
//	
//	else
//	{
//		
//		new_element->next = element;
//		element->next =new_element;
//		
//	}
//	
//		list->size +=1;
//		return 0;
//
//}


ListElmt *find(const List *list, int value)
{
	int i,data1; 
	char select; 
	int *data;
	ListElmt *old_element;
	
	ListElmt *pt = list -> head;
	
			
	i=0;
	while(1)
	{
			
		if(*((int *)(pt->data))==value)
			{
			printf("�� %d�� ���� %d��° ��忡 ����Ǿ��ֽ��ϴ�.\n",value,i+1);
	
			return pt;
			}
				
			break; 
		
			}			
		i+=1;		
		pt = pt -> next;
		
		if(pt==NULL) 
		{
		
		printf("�Է°��� ��ġ�ϴ� ��带 ã�� ���Ͽ����ϴ�.\n",value);
		return 0;
		}
		
	}
				
	



// int main ()
// {
// 	List list;
// 	int i,value,data1;
// 	int *data;
// 	char select;
	
		
// 	ListElmt *element;

	
// 	list_init(&list,free);
	

// 	while(1) 
// 	{
		
// 		printf("[========menu========]\n");
// 		printf("1.insert data\n");
// 		printf("2.printf data\n");
// 		printf("3.find node\n");
// 		printf("> ��ȣ ����<0:����>: ");
// 		scanf("%d",&i);
	
	
	
// 	if(i==0) break;
	
	
// 	switch (i)
// 	{
// 		case 1 : 
		
// 		while(1)
// 		{
		
// 		printf("������ �� �Է� :");
		
// 		if ((data = (int *)malloc(sizeof(int))) == NULL)
// 			return -1;
// 		scanf("%d",&value);
		
// 		*data=value;
		
// 		list_ins_next(&list,NULL,data);
		
// 		printf("�� �Ͻðڽ��ϱ�<N:exit>");
		
// 		fflush(stdin);
// 		scanf("%c",&select);

// 		if(select == 'N') break;
		
// 		}
// 		break;
				
// 		case 2 : 
		
// 			print_list(&list);
// 			break;	
		
// 		case 3 : 
// 			printf("ã�� ���� �Է��Ͻÿ� : ");
		
// 			scanf("%d",&i); 
		 
// 			element=find(&list,i);
			
// 			printf("�� ��� ������ ���� �����ðڽ��ϱ�?<Y:yes>");
// 			fflush(stdin);
// 			scanf("%c",&select); 
		
// 			if(select == 'Y')
// 			{
			
// 			printf("������ �� �Է� : ");
// 			scanf("%d",&data1);
// 			fflush(stdin);
			
// 			if ((data = (int *)malloc(sizeof(int))) == NULL)
// 			return -1;
						
// 			*data = data1;			
						
// 			list_ins_next(&list,element,data);
			
			
// 			break;	
		
// 	}
	
// }
// }
// }
		
	

	
  






