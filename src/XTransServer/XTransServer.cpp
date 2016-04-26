#include "StdAfx.h"
#include "XServer.h"
#include "XTransServer.h"

int main(int argc, char* argv[])
{
	if (DoInitialize() != X_SUCCESS)
		return X_FAILURE;

	while(true)
	{
		uv_run(uv_default_loop(), UV_RUN_DEFAULT);
		uv_sleep(INFINITE);//如果uv_run没有Block，则由sleep来阻塞主线程。
	}

	exit(0);
	return X_FAILURE;
}
