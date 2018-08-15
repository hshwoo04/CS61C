
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "utils.h"
#include "tables.h"

/*******************************
 * Helper Functions
 *******************************/

void allocation_failed() {
    write_to_log("Error: allocation failed\n");
    exit(1);
}

void addr_alignment_incorrect() {
    write_to_log("Error: address is not a multiple of 4.\n");
}

void name_already_exists(const char* name) {
    write_to_log("Error: name '%s' already exists in table.\n", name);
}

void write_symbol(FILE* output, uint32_t addr, const char* name) {
    fprintf(output, "%u\t%s\n", addr, name);
}

void passed_null_table() {
	write_to_log("Error: passed a null table.\n");
}

/*******************************
 * Symbol Table Functions
 *******************************/

/* Creates a new SymbolTable containing 0 elements and returns a pointer to that
   table. Multiple SymbolTables may exist at the same time. 
   If memory allocation fails, you should call allocation_failed(). 
 */
SymbolTable* create_table() {
	SymbolTable* retval;

	/* Allocate memory for the struct */
	retval = (SymbolTable*)malloc(1 * sizeof(SymbolTable)); 

	/* Check return value to make sure memory allocation worked */
	if(retval == NULL) {
		allocation_failed();
	}

	/* Initialize elements inside SymbolTable */
	retval->len = 1;
	retval->cap = 4;
	retval->tbl = (Symbol**)malloc( (1 + (retval->cap)) * sizeof(Symbol*));

	/* Check retval again to make sure mem alloc worked */
	if(retval->tbl == NULL) {
		free(retval);
		allocation_failed();
	}

	/* allocate memory for Symbol */
	Symbol *sym;
	sym = (Symbol*)malloc(1 * sizeof(Symbol));
	
	/* make sure allocation worked */
	if(sym == NULL) {
		allocation_failed();
	}

	/* allocate memory for char* name */
	sym->addr = 0;
	sym->name = (char*)malloc(10 * sizeof(char));

	/* make sure allocation worked */
	if(sym->name == NULL) {
		allocation_failed();
	}

	retval->tbl[0] = sym;

    return retval;
}

/* Frees the given SymbolTable and all associated memory. */
void free_table(SymbolTable* table) {
	int length = table->len;
	for (int i = 0; i < length; i += 1) {
		free(table->tbl[i]->name);
	}
    free(table->tbl);
    free(table);
}

/* Adds a new symbol and its address to the SymbolTable pointed to by TABLE. 
   ADDR is given as the byte offset from the first instruction. The SymbolTable
   must be able to resize itself as more elements are added. 

   Note that NAME may point to a temporary array, so it is not safe to simply
   store the NAME pointer. You must store a copy of the given string.

   If ADDR is not word-aligned, you should call addr_alignment_incorrect() and
   return -1. If NAME already exists in the table, you should call 
   name_already_exists() and return -1. If memory allocation fails, you should
   call allocation_failed(). 

   Otherwise, you should store the symbol name and address and return 0.
 */
int add_to_table(SymbolTable* table, const char* name, uint32_t addr) {
	
	/* Check to make sure ADDR is word-aligned (multiple of 4) */
	if (addr % 4 != 0) {
		addr_alignment_incorrect();
		return -1;
	}

	/* Check to make sure table is not NULL */
	if(table == NULL) {
		passed_null_table();
	}

	/* Store copy of given string */
	char* p_name = (char*)malloc(10 * sizeof(char)); 
	strcpy(p_name, name);

	/* If tbl length is less than the cap, then add the new symbol 
	 * should only pass through this during the first entry*/
	if(table->len < table->cap) {
		int length = table->len;
		/* copy contents of tbl over into new memory */
		
		for(int i = 0; i < length; i += 1) {
			Symbol* sym = table->tbl[i];
			char* name_sym = sym->name;
			if (strcmp(name_sym, p_name) == 0) {
				name_already_exists(p_name);
				return -1;
			}
		}
		
		/* allocate memory for Symbol */
		Symbol *retval;
		retval = (Symbol*)malloc(1 * sizeof(Symbol));
		
		/* make sure allocation worked */
		if(retval == NULL) {
			allocation_failed();
		}

		/* allocate memory for char* name */
		retval->addr = 0;
		retval->name = (char*)malloc(10 * sizeof(char));

		/* make sure allocation worked */
		if(retval->name == NULL) {
			allocation_failed();
		}

		/* initialize data in Symbol */
		retval->addr = addr;
		retval->name = p_name;

		/* add Symbol */
		table->tbl[length] = retval;
		table->len = length + 1;

		return 0;
	}
	/* resize tbl by malloc (table->cap) + 1, and add new symbol entry */
	else {
		/* create new memory that is one larger than current tbl */
		int capacity = table->cap;
		int length = table->len;
		Symbol** temp_tbl = (Symbol**)malloc((1+capacity) * sizeof(Symbol*));
		/* copy contents of tbl over into new memory */
		for(int i = 0; i < length; i += 1) {
			/* check for duplicates */
			Symbol* sym = table->tbl[i];
			char* name_sym = sym->name;
			if (strcmp(name_sym, p_name) == 0) {
				name_already_exists(p_name);
				return -1;
			}
			temp_tbl[i] = table->tbl[i];
			
		}

		free(table->tbl);

		/* allocate memory for Symbol */
		Symbol *retval;
		retval = (Symbol*)malloc(1 * sizeof(Symbol));
		
		/* make sure allocation worked */
		if(retval == NULL) {
			allocation_failed();
		}

		/* allocate memory for char* name */
		retval->addr = 0;
		retval->name = (char*)malloc(10 * sizeof(char));

		/* make sure allocation worked */
		if(retval->name == NULL) {
			allocation_failed();
		}

		/* initialize data in Symbol */
		retval->addr = addr;
		retval->name = p_name;

		/* add Symbol */
		temp_tbl[length] = retval;
		table->tbl = temp_tbl;
		table->len = length + 1;
		table->cap = capacity + 1;

		return 0;
	}


    return -1;
}

/* Returns the address (byte offset) of the given symbol. If a symbol with name
   NAME is not present in TABLE, return -1.
 */
int64_t get_addr_for_symbol(SymbolTable* table, const char* name) {
    int length = table->len;
	for (int i = 0; i < length; i += 1) {
		Symbol* sym = table->tbl[i];
		char* name_sym = sym->name;
		if (strcmp(name_sym, name) == 0) {
			return sym->addr;
		}
	}

    return -1;   
}

/* Writes the SymbolTable TABLE to OUTPUT. You should use write_symbol() to
   perform the write. Do not print any additional whitespace or characters.
 */
void write_table(SymbolTable* table, FILE* output) {
    int length = table->len;
    for (int i = 0; i < length; i += 1) {
    	Symbol* cur_sym = table->tbl[i];
    	if (cur_sym != NULL) {
    		if(cur_sym->addr == 0 && cur_sym->name[0] == '\0') {
    			continue;
    		}
    		write_symbol(output, cur_sym->addr, cur_sym->name);
    	}
    }
}
