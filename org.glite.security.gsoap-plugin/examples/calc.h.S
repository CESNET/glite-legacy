//gsoap egeesec schema namespace:   urn:calc
//
//gsoap egeesec service name:       calc
//gsoap egeesec service port:       http://localhost:9999/
//gsoap egeesec service namespace:  urn:calc:wscalc

//gsoap egeesec service method-style:       add rpc
//gsoap egeesec service method-action:      add ""
int wscalc__add(double a, double b, double *result);

//gsoap egeesec service method-style:       sub rpc
//gsoap egeesec service method-action:      sub ""
int wscalc__sub(double a, double b, double *result);
