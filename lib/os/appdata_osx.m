#include <stdio.h>
#include "../../include/os.h"
#include <string.h>
#include <Foundation/NSFileManager.h>
#include <Foundation/NSPathUtilities.h>
#include <Foundation/NSURL.h>
#include <Foundation/NSAutoreleasePool.h>
#include <Foundation/NSBundle.h>

static char path[128];

static void setworkdir(void);

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

	// while we're in OSX-land...
	setworkdir();

	[pool release];
	return path;
}

static void setworkdir(void){
	NSBundle *b = [NSBundle mainBundle];
	if(!b)
		return;

	NSURL *burl = [b bundleURL];
	NSURL *xurl = [burl URLByAppendingPathComponent:@"Contents/MacOS"];

	NSString *apath = [xurl path];
	NSFileManager *fm = [NSFileManager defaultManager];
	[fm changeCurrentDirectoryPath:apath];
}
