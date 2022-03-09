/* a small POSIX-like date implementation
 *
 * (C) 2004 Ruediger Sonderfeld <kingruedi@c-plusplus.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <time.h>

// I know define is bad but it saved 32byte!
#define OUTPUT_SIZE 201

int main(int argc, char **argv) {
  const char *format="%a %b %d %H:%M:%S %Z %Y";
  time_t t=time(NULL);
  struct tm *ptr=NULL;
  int i;
  char output[OUTPUT_SIZE];
  size_t output_write;
  //check options
  if(argc>1) {
    for(i=1;i<argc;++i) {
      if(argv[i][0]=='-') {
	//if(argv[i][1]=='u') { // date only takes u with '-' prefix as option
	//-u option: timezone == UTC0
	ptr=gmtime(&t);
	// }
      }
      else if(argv[i][0]=='+') {
	//user defined format
	format=argv[i]+1;
      }
    }
  }
  if(!ptr)
    ptr=localtime(&t);
  output_write=strftime(output,OUTPUT_SIZE,format,ptr);
  write(1,output,output_write);
  write(1,"\n",1);

  return 0;
}
