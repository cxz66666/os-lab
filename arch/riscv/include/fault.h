#ifndef _FAULT_H
#define _FAULT_H

#define CAUSE_FETCH_PAGE_FAULT 12
#define CAUSE_LOAD_PAGE_FAULT 13
#define CAUSE_STORE_PAGE_FAULT 15

void do_page_fault(int scause);
#endif
