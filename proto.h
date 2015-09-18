#include <rpc/types.h>

/****** rpc_clntout.c ******/

void write_stubs(void);
void printarglist(proc_list *proc, char *result,
		  char *addargname, char *addargtype);

/****** rpc_cout.c ******/

void emit (definition *def);

/****** rpc_hout.c ******/

void print_datadef(definition *def);
void print_funcdef(definition *def);
void pxdrfuncdecl(char *name, int pointerp);
void pprocdef(proc_list *proc, version_list *vp,
	      char *addargtype, int server_p, int mode);
void pdeclaration(char *name, declaration *dec, int tab,
		  char *separator);
void print_xdr_func_def (char* name, int pointerp, int i);

/****** rpc_main.c ******/
	/* nil */

/****** rpc_parse.c ******/
definition *get_definition(void);

/****** rpc_sample.c ******/
void write_sample_svc(definition *def);
int write_sample_clnt(definition *def);
void add_sample_msg(void);
void write_sample_clnt_main(void);

/****** rpc_scan.c ******/
   /* see rpc_scan.h */

/****** rpc_svcout.c ******/
int nullproc(proc_list *proc);
void write_svc_aux(int nomain);
void write_msg_out(void);

/****** rpc_tblout.c ******/
void write_tables(void);

/****** rpc_util.c ******/
void reinitialize(void);
int streq(char *a, char *b);
void error(char *msg) __attribute__ ((noreturn));
void crash(void) __attribute__ ((noreturn));
void tabify(FILE *f, int tab);
char *make_argname(char *pname, char *vname);
void add_type(int len, char *type);
