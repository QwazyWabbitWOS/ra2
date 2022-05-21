#include "g_local.h"
/************* MENU.C
Menu Diagram (client->menulist) 
    (prev)        (prev)                      (prev)              (next)
null ---- menulist ----  menulink1  ---------------  menulink2  -------------  null
           |it    (next)  |(it)     (next)            |(it)     (next)           
          null           menu1  -----  null          menu2  -----  null           
                          |(next) (it)                |(next) (it)                
                    (prev)|                           |                           
                         itemlink1 --  menuitem1     itemlink1 --  menuitem1      
                          |(next) (it)  |-itemtext    |(next) (it)  |-itemtext    
                          |             |-ItemSelect  |             |-ItemSelect  
                    (prev)|                           |                           
          		       itemlink2 --  menuitem2       itemlink2 --  menuitem2       
                          |             |-itemtext    |             |-itemtext    
          		       null             |-ItemSelect null           |-ItemSelect   
                                                    

******************/

#ifndef ARENA

/*******
add_to_queue
*******/
void add_to_queue(arena_link_t *t, arena_link_t *que)
{
        while(que->next)
                que=que->next;
        que->next = t;
        t->prev = que;
        t->next = NULL;
}

/*******
remove_from_queue

if NULL is given as first parameter, top list item is popped off

item that is removed is returned, or NULL if not found
*******/
arena_link_t *remove_from_queue(arena_link_t *t, arena_link_t *que)
{
        arena_link_t *got = NULL;

        if(!t) t = que->next;
        if(!t) return(NULL);
        t->prev->next = t->next;
        if(t->next)
                t->next->prev = t->prev;

        return(t);
}

/*******
count_queue
*******/
int count_queue(arena_link_t *que)
{
        int i = 0;

        while(que->next) {
                que=que->next;
                i++;
        }

        return(i);
}

#endif 
void PrintMenuItem(menu_item_t *menuitem)
{
	gi.bprintf(PRINT_HIGH, "  %s %s %d\n", menuitem->itemtext, menuitem->valuetext, menuitem->itemvalue);
}

void PrintMenu(arena_link_t *menulink)
{
	arena_link_t	*que;

	que=menulink->it;
	gi.bprintf(PRINT_HIGH, "%s\n", ((arena_link_t *)menulink->it)->it);
	while(que->next) {
                que=que->next;
                PrintMenuItem(que->it);
        }

 
}

void PrintMenuQueue(edict_t *ent)
{
	arena_link_t	*que;

	que=&ent->client->menulist;
    while(que->next) {
                que=que->next;
                PrintMenu(que);
        }
}

char *LoPrint(char *text)
{
	int i;

	if (!text)
		return NULL;
	for (i=0; i<strlen(text) ; i++)
		if ((byte)text[i] > 127)
			text[i]=(byte)text[i]-128;

	return text;
}

char *HiPrint(char *text)
{
	int i;

	if (!text)
		return NULL;
	for (i=0; i<strlen(text) ; i++)
		if ((byte)text[i] < 127 && (byte)text[i] >=32)
			text[i]=(byte)text[i]+128;
	return text;
}

#define FANCY_MENUS

void SendMenu(edict_t *ent)
{
	gi.WriteByte (0x0D); //configstring
	gi.WriteShort(5); //status bar index
	gi.WriteString (ent->client->menudata);
	gi.unicast (ent, false);
	//gi.dprintf("sendmenu: %d\n",level.framenum);
}

void SendStatusBar(edict_t *ent, char *bar,qboolean force)
{
	//gi.dprintf("sendstatusbar: %d\n",level.framenum);
	strncpy(ent->client->menudata,bar,1400);
	ent->client->menuframe = level.framenum+1;
	if (force)
	{
		if (ent->client->menuframe != level.framenum)
			SendMenu(ent);
		ent->client->menuframe = level.framenum;
	}
	else
		ent->client->menuframe = level.framenum+1;
	

}

extern char *dm_statusbar; //in g_spawn.c
extern char *single_statusbar;

void DisplayMenu(edict_t *ent)
{
	arena_link_t *cur, *menu, *selected;
	char my_statusbar[1400];
	char tempitem[1000];
	char *pos;
	int i,y;
	
#ifndef FANCY_MENUS
	DisplaySimpMenu(ent);
	return;
#endif
	if (!ent->client->showmenu) //clear the old one
	{
		if (deathmatch->value)
			SendStatusBar(ent, dm_statusbar,true);
		else
			SendStatusBar(ent, single_statusbar,true);

		
		return;
	}

	menu=ent->client->curmenulink->it;
	selected=ent->client->selected;

	my_statusbar[0]='\0';
	sprintf (my_statusbar, 
		"xv 32 yv 8 picn inventory ");			// background
	
	pos = my_statusbar+strlen(my_statusbar);
	
	sprintf(pos,
		"xv 202 yv 12 string2 \"%s\" ",		// menu 
		"Menu");
	
	pos = my_statusbar+strlen(my_statusbar);
	
	sprintf(pos,
		"xv 0 yv 24 cstring2 \"%s\" ",		// menu title
		menu->it);

	pos = my_statusbar+strlen(my_statusbar);

//if we have too many to display
	i=count_queue(menu) - count_queue(selected);
	if (i>18)
	{
		cur=selected;
		do
		{
			cur=cur->prev; 
			i--;
		} while (cur != menu && i % 18 != 0);
		sprintf(pos, "xv 50 yv 32 string2 \"(More)\" ");
	} else
	{
		cur = menu;
		sprintf(pos, "xv 50 ");
	}
	pos = my_statusbar+strlen(my_statusbar);

	y = 32;
	i=0;
	while(cur->next && i<18)  //only display 18 items
		{ //add items
				cur=cur->next;
				y+=8;
				i++;
				tempitem[0]='\0';
                if (cur == selected) 
				{
					strcat(tempitem, "\15"); //" > " char
					strcat(tempitem, LoPrint(((menu_item_t *)cur->it)->itemtext));
					if (((menu_item_t *)cur->it)->valuetext)
						strcat(tempitem, ((menu_item_t *)cur->it)->valuetext);
				}
				else
				{
					strcat(tempitem, " ");
					strcat(tempitem, HiPrint(((menu_item_t *)cur->it)->itemtext));
					if (((menu_item_t *)cur->it)->valuetext)
						strcat(tempitem,((menu_item_t *)cur->it)->valuetext);
				}

				LoPrint(((menu_item_t *)cur->it)->itemtext); //reset it back to the normal state


				if (((menu_item_t *)cur->it)->itemvalue >= 0)
					sprintf(tempitem + strlen(tempitem), "%d", ((menu_item_t *)cur->it)->itemvalue);

			
				if (strlen(my_statusbar)+strlen(tempitem)+50 >= 1400)
					break;
				sprintf(pos,"yv %d string2 \"%s\" ",		// each menu item
					y, tempitem);
				pos = my_statusbar+strlen(my_statusbar);
				
        }
	if (i==18 && cur->next) //if there are more
		sprintf(pos, "yv %d string2 \"(More)\" ", y+10);
	//gi.dprintf("Length: %d\n", strlen(my_statusbar));
	SendStatusBar(ent, my_statusbar,false);
	
}

void DisplaySimpMenu(edict_t *ent)
{
	char	string[1400];
	int		total;
	arena_link_t *cur, *menu, *selected;


	if (!ent->client->showmenu) //clear the old one
	{
		gi.centerprintf(ent, "");
		return;
	}
	menu=ent->client->curmenulink->it;
	selected=ent->client->selected;
	
	total = count_queue(menu); // get the menu count
	string[0] = 0;
	strcat(string, HiPrint(menu->it)); //print title
	LoPrint(menu->it); //reset it back to the normal state
	strcat(string, "\n"); //space b/t title and items

	cur = menu;
	while(cur->next) 
		{ //add items
				cur=cur->next;
				strcat(string, "\n");
                if (cur == selected)
					strcat(string, "*");
				strcat(string, ((menu_item_t *)cur->it)->itemtext);
				if (((menu_item_t *)cur->it)->valuetext)
					strcat(string, ((menu_item_t *)cur->it)->valuetext);
				if (((menu_item_t *)cur->it)->itemvalue >= 0)
					sprintf(string + strlen(string), "%d", ((menu_item_t *)cur->it)->itemvalue);

				
        }


	gi.centerprintf(ent, "%s", string);
}

/*void *giTagMalloc(int size, int level)
{
	return malloc(size);

}

void giTagFree(void *ptr)
{
	free(ptr);
}
*/
arena_link_t *CreateQMenu(edict_t *ent, char *name)
{
	arena_link_t	*menu;
	arena_link_t	*menulink;

	menu = gi.TagMalloc(sizeof(arena_link_t), TAG_LEVEL);
	menulink = gi.TagMalloc(sizeof(arena_link_t), TAG_LEVEL);
	menulink->it = menu;
	menu->it = gi.TagMalloc(strlen(name)+1, TAG_LEVEL);
	strcpy(menu->it,name);
	menu->next = menu->prev = NULL;
	return menulink;
}

arena_link_t *AddMenuItem(arena_link_t *menulink, char *itemtext, char *valuetext, int value, void *Callback)
{
	arena_link_t	*itemlink;
	menu_item_t	*iteminfo;

	itemlink = gi.TagMalloc(sizeof(arena_link_t), TAG_LEVEL);
	iteminfo = gi.TagMalloc(sizeof(menu_item_t), TAG_LEVEL);
//arena
	iteminfo->itemtext = gi.TagMalloc(strlen(itemtext)+1, TAG_LEVEL);
	strcpy(iteminfo->itemtext, itemtext);
	if (valuetext)
	{
		iteminfo->valuetext = gi.TagMalloc(strlen(valuetext)+1, TAG_LEVEL);
		strcpy(iteminfo->valuetext, valuetext);
	} else
		iteminfo->valuetext = NULL;
	iteminfo->itemvalue = value;
	iteminfo->ItemSelect = Callback; //ItemSelect;
	itemlink->it = iteminfo;
	add_to_queue(itemlink, menulink->it); //it is the actualy menu
	return itemlink;

}

void FinishMenu(edict_t *ent, arena_link_t *menulink,int showit)
{

	ent->client->curmenulink = menulink;
	ent->client->selected = ((arena_link_t *)menulink->it)->next; //it is menu, it->next is first item
	ent->client->showmenu = showit;
	add_to_queue(menulink, &ent->client->menulist);
	DisplayMenu(ent);
}


void MenuNext(edict_t *ent)
{
	if (ent->client->selected->next)
	{
		ent->client->selected=ent->client->selected->next;
	
//skip over blank ones
	while (ent->client->selected->next && !(((menu_item_t *) ent->client->selected->it)->ItemSelect))
		ent->client->selected=ent->client->selected->next;
	}
	else
		ent->client->selected=((arena_link_t *)ent->client->curmenulink->it)->next; //first item
	DisplayMenu(ent);
}

void MenuPrev(edict_t *ent)
{
	if (ent->client->selected->prev->prev)
	{
		ent->client->selected=ent->client->selected->prev;
//skip over blank ones
	while (ent->client->selected->prev->prev && !(((menu_item_t *) ent->client->selected->it)->ItemSelect))
		ent->client->selected=ent->client->selected->prev;
	}
	else
		while (ent->client->selected->next)
			ent->client->selected=ent->client->selected->next; //go to the last
	DisplayMenu(ent);
}
	

void UseMenu(edict_t *ent, int key) //key=1 invuse, key=0 invdrop
{
	arena_link_t	*que,*item;
	int ret;

	if (ent->client->menuselectframe + 5 > level.framenum) //0.5 sec timer between selections.. stops double click syndrome
		return;
	ent->client->menuselectframe = level.framenum;
	que=ent->client->curmenulink;
	if (((menu_item_t *) ent->client->selected->it)->ItemSelect) //if there is a callback
	{
		if (ret=((menu_item_t *) ent->client->selected->it)->ItemSelect(ent,que, ent->client->selected, key)) //if the callback didnt return 0, leave
		{
			if (ret == 1)
				DisplayMenu(ent); //only display the update if it returns 1
			return;
		}
	} else //if there was no callback, leave on stack
		return;
	
	remove_from_queue(que, &ent->client->menulist);

	item=que->it;
	gi.TagFree(item->it); //arena name
	while (item->next)
	{
		item=item->next;
		gi.TagFree(((menu_item_t *)(item->it))->itemtext);
		if (((menu_item_t *)(item->it))->valuetext)
			gi.TagFree(((menu_item_t *)(item->it))->valuetext);
		if (item->prev)
			gi.TagFree(item->prev);
	}
	if (item)
		gi.TagFree(item);
	gi.TagFree(que);

	que=&ent->client->menulist; //go to front of list

	while (que->next)
		que=que->next; //find last item
	
	if (que->it) //if there  menu
	{
		
		ent->client->curmenulink = que;
		ent->client->selected=((arena_link_t *)ent->client->curmenulink->it)->next; //first menu item link
		
	}
	else  
	{//modified for arena, this way we can turn the menu off and not have it turn back on
		ent->client->curmenulink = NULL;
		ent->client->showmenu = 0;
	}


//modified for arena
//	ent->client->showmenu = ent->client->curmenulink ? true : false;
	DisplayMenu(ent);
}


qboolean MenuThink(edict_t *ent)
{
//ifndef FANCY_MENUS 
	//-- modified for arena, keep sending menu to get around overflow
	
//	gi.dprintf("menuthink: %d\n",level.framenum);
	if (ent->client->showmenu &&  (level.framenum - ent->client->menuframe) % 10 == 0)
	{
		SendMenu(ent);
		return true;
	} else
			return false;
//endif

}

/************
clear_menus  - gets rid of any menus
***********/
void clear_menus(edict_t *ent)
{
	ent->client->showmenu=0;
	ent->client->curmenulink=NULL;
	ent->client->menulist.next=NULL;
	DisplayMenu(ent);
}

/**************
Example Stuff
***************/

int MySelect(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key)
{

	gi.bprintf(PRINT_HIGH, "menu item %s selected by %s\n", ((menu_item_t *)selected->it)->itemtext, ent->client->pers.netname);
	return 0;
}

int MySelect2(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key)
{
	if (key) //inc for enter, dec for '
		((menu_item_t *)selected->it)->itemvalue++;
	else
		((menu_item_t *)selected->it)->itemvalue--;

	if (!((menu_item_t *)selected->it)->itemvalue) //if we went to zero
		((menu_item_t *)selected->it)->itemvalue=1;
	return 1;

}

int MySelect3(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key)
{
	char fl[8], tl[8];
	arena_link_t *itemlink;

	itemlink=((arena_link_t *)(menulink->it) )->next; //first menuitemlink on the current menu
	sprintf(fl, "%d", ((menu_item_t*) (itemlink->it))->itemvalue);
	itemlink=itemlink->next; //next menuitemlink
	sprintf(tl, "%d", ((menu_item_t*) (itemlink->it))->itemvalue);

	gi.bprintf(PRINT_HIGH, "Fraglimit is now %s. Timelimit is now %s\n", fl, tl);
	gi.cvar_set("fraglimit",fl);
	gi.cvar_set("timelimit",tl);
	return 0;
}


#ifndef ARENA
void menu_test_f(edict_t *ent)
{
	arena_link_t *menu1l, *menu2l, *menu3l;
	char *menuitem;
	int i;

	menu1l=CreateQMenu(ent, "More Options");
	AddMenuItem(menu1l, "menu1item1",NULL, -1,  &MySelect);
	AddMenuItem(menu1l, "menu1item2",NULL, -1, &MySelect);
	AddMenuItem(menu1l, "menu1item3",NULL, -1, &MySelect);
	AddMenuItem(menu1l, "menu1item4",NULL, -1, &MySelect);
	FinishMenu(ent, menu1l,1);	

	menu2l=CreateQMenu(ent, "Options");
	AddMenuItem(menu2l, "Fraglimit - ",NULL, 1, &MySelect2);
	AddMenuItem(menu2l, "Timelimit - ",NULL, 1, &MySelect2);
	AddMenuItem(menu2l, "OK", NULL, -1, &MySelect3);
	FinishMenu(ent, menu2l,1);


	menu3l=CreateQMenu(ent, "Big List");
	for (i=0; i<30; i++)
	{
		AddMenuItem(menu3l, va("Menuitem %d---------",i), NULL, -1, &MySelect);
	}
	FinishMenu(ent,menu3l,1);

//	PrintMenuQueue(ent);

	
}
#endif