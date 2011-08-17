#include <stdio.h>
#include "../../include/os.h"
#include <string.h>
#include <Foundation/NSFileManager.h>
#include <Foundation/NSPathUtilities.h>
#include <Foundation/NSURL.h>

static char path[128];

const char *appdata(const char *prog){
	if(path[0] != 0)
		return path;

	NSFileManager *fm = [NSFileManager defaultManager];
	NSArray *urls = [fm URLsForDirectory:NSApplicationSupportDirectory inDomains:NSUserDomainMask];

	if([urls count] == 0)
		return NULL;

	NSString *nsprog = [NSString stringWithCString:prog encoding:NSUTF8StringEncoding];

	NSURL *adpath = [urls objectAtIndex:0];
	NSURL *upath = [adpath URLByAppendingPathComponent:nsprog];

	const char *p = [[upath absoluteString] UTF8String];
	strncpy(path, p, sizeof(path)-1);
	return path;
}
