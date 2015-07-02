#include <rpc/rpc.h>
#include "rdict.h"

static int ret;

int* insertw_1_svc(char **w, struct svc_req *rqstp)
{
  ret = insertw(*w);
  return &ret;
}

int* initw_1_svc(void *p, struct svc_req *rqstp)
{
  ret = initw();
  return &ret;
}

int* deletew_1_svc(char **w, struct svc_req *rqstp)
{
  ret = deletew(*w);
  return &ret;
}

int* lookupw_1_svc(char **w, struct svc_req *rqstp)
{
  ret = lookupw(*w);
  return &ret;
}
