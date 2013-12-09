#include "tinyFS.h"
#include "libTinyFS.h"
#include "TinyFS_errno.h"

int main() {
   int status = 42, status2 = 41, status3 = 40;
   int i;
   fileDescriptor fd, fd2, fd3;
   /* the following blah was a funny thing taken from the internet */
   char *blah = "what the fuck did you just fucking say about me, you little bitch? i'll have you know i graduated top of my class in the navy seals, and i've been involved in numerous secret raids on al-quaeda and i have over 300 confirmed kills, i am trained in gorilla warfare and i'm the top sniper in the entire us armed forces. you are nothing to me but just another target. i will wipe you the fuck out with precision the likes of which has never been sseen before on this earth, m ark my fucking words. you think you can get away with saying that shit to me over the internet? think again, fucker. as we speak i am contacting my secret network of spies across the USA and your IP is being traced right now so you better prepfare for the storm, maggot. the storm that wipes out the pathetic little thin you call your life. you're fucking dead, kids. i can be anywhere, antytime, and i can kill you in over seven hundred ways, and that's just with my bare hands, no tonly am i extensitively trained in unarmed combat, but i have access to the entire arsenal of the united states marine corps and i will use it to its full extend to wipe your miserable ass off the face of the continent, you little shit. if you could have known what unholy retribution your little clverer comment was about to bring down upon you, maybe you would have held your fucking tongue,, but you couldn't";
   char *blah2 = "tiny message";
   char datByte;

   status = tfs_mkfs("asdf", DEFAULT_DISK_SIZE);
   status3 = tfs_mount("asdf");
   fd = tfs_openFile("filez");
   fd2 = tfs_openFile("filez2");
   tfs_makeRO("filez");
   status2 = tfs_writeFile(fd, blah, 1300);

   printf("after makeRO: %d\n", status2);
   
   /*fd3 = tfs_openFile("filez3");
   */
   //status2 = tfs_writeFile(fd, blah, 1300);

   tfs_makeRW("filez");
   status2 = tfs_writeFile(fd, blah, 1300);
   printf("after makeRW: %d\n", status2);
   status2 = tfs_seek(fd, 1299);
   status2 = tfs_writeByte(fd, 'g');
   printf("after writeByrte: %d\n", status2);
   /*
   time_t a = tfs_readFileInfo(fd);
   printf("THE TIME: %s\n", ctime(&a));
   */
   
   //status3 = tfs_deleteFile(fd2);
   //status2 = tfs_writeFile(fd3, blah2, sizeof("tiny message"));
   

   /*for(i = 0; i < 1300; i++) {
      tfs_readByte(fd, &datByte);
      printf("%c", datByte);
   }*/
   printf("\n");

   printf("tfs_mkfs: %d\n tfs_mount %d tfs_openfile: %d\n", status, status3, status2);

   /*
   int r = openDisk("asdf", 10240), status;
   void *a = "asdfasdf\0", *b;
//   printf("%d\n", r);
   status = writeBlock(r, 0, a);
//   printf("%d\n", status);
   b = malloc(256);
   status = readBlock(r, 0, b);
   printf("%d\n%s\n", 123234782578, (char*)b);
   
   
   free(b);
   close(r);
   */
   return 0;
}
