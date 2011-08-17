#include <stdio.h>
#include "../../include/os.h"
#include <string.h>
#include <Foundation/NSFileManager.h>
#include <Foundation/NSPathUtilities.h>
#include <Foundation/NSURL.h>
#include <Foundation/NSAutoreleasePool.h>

static char path[128];

const char *appdata(const char *prog){
	if(path[0] != 0)
		return path;

	//TODO: this should go somewhere else!
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

	NSFileManager *fm = [NSFileManager defaultManager];
	NSArray *urls = [fm URLsForDirectory:NSApplicationSupportDirectory inDomains:NSUserDomainMask];

	if([urls count] == 0){
		[pool release];
		return NULL;
	}

	NSString *nsprog = [NSString stringWithCString:prog encoding:NSUTF8StringEncoding];

	NSURL *adpath = [urls objectAtIndex:0];
	NSURL *upath = [adpath URLByAppendingPathComponent:nsprog];

	const char *p = [[upath path] UTF8String];
	strncpy(path, p, sizeof(path)-1);

	[pool release];
	return path;
}
