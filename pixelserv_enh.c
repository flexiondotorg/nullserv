/*
**    pixelserv.c
**    http://www.linuxquestions.org/questions/programming-9/pixelserv-with-xinetd-763702/
**         
**    build: make pixelserv
**    run:   pixelserv
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static const unsigned char gifpixel[] = 
    "HTTP/1.1 200 OK\r\n"
    "Content-type: image/gif\r\n"
    "Accept-ranges: bytes\r\n"
    "Content-length: 43\r\n"
    "Connection: close\r\n"
    "\r\n"
    "GIF89a\1\0\1\0\200\0\0\377\377\377\0\0\0\41\371\4\1\0\0\0\0\54\0\0\0\0\1\0\1\0\0\2\2\104\1\0;" ;

#define TOKEN_SIZE 4096
#define DEFAULT_MIMETYPE "text/plain"

// The client request
char request[TOKEN_SIZE];

// Whether the stream is exhausted after reading the HTTP headers
int exhausted = 0;

// Read the HTTP headers and translate them to 
// environment variables. Stop reading when we
// get a blank line so we know anything after
// that is form data.
void read_http_headers() {

    char buf[TOKEN_SIZE];
    char* name;
    char* value;
    char* env;
    int i;
    while (1) {

        // Read the next header, if it couldn't be read, mark
        // the stream as exhausted so we don't try to get
        // any more
        if (!fgets(buf, sizeof(buf), stdin)) {
            #ifdef LOGGING
                syslog(LOG_MAKEPRI(LOG_DAEMON, LOG_DEBUG), "Stream is exhausted, no more data");
            #endif
            exhausted = 1;
            break;
        }

        // Stop reading headers if we get a blank line
        if (*buf == '\r' || *buf == '\n' || *buf == '\0') {
            #ifdef LOGGING
                syslog(LOG_MAKEPRI(LOG_DAEMON, LOG_DEBUG), "Got blank line - end of HTTP headers (%s)", buf);
            #endif
            break;
        }

        #ifdef LOGGING
            syslog(LOG_MAKEPRI(LOG_DAEMON, LOG_DEBUG), "Got HTTP header: %s", buf);
        #endif

        // Split the line into name/value
        name = strtok(buf, ":");
        value = strtok(NULL, "\r");

        // Remove leading space
        if (*value == ' ') value++;

        // Swap any hyphens in the name for underscores and
        // upper case any letters
        for (i = 0; i < strlen(name); i++)
            if (*(name + i) == '-') 
                (*(name + i)) = '_';
            else
                (*(name + i)) = toupper(*(name + i));

        // Allocate the environment variable string dynamically
        // as each memory block pointed to by env becomes
        // part of the environment and we don't want to
        // overwrite vars (no, we don't need to free it)
        env = (char*) malloc(TOKEN_SIZE);
        snprintf(env, TOKEN_SIZE, "HTTP_%s=%s", name, value);
        //printf("HTTP_%s=%s\r\n", name, value);                    
        putenv(env);

        #ifdef LOGGING
            syslog(LOG_MAKEPRI(LOG_DAEMON, LOG_DEBUG), "CGI env variable: %s", env);
        #endif
    }

}

char* get_mime_type(char* name)
{
    // This is really crude, but quick. Could probably use
    // libmagic for accuracy
    char* ext = strrchr(name, '.');
    if (!ext) return DEFAULT_MIMETYPE;
    if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0) return "text/html";
    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0 || strcmp(ext, ".JPG") == 0) return "image/jpeg";
    if (strcmp(ext, ".txt") == 0) return "text/plain";
    if (strcmp(ext, ".csv") == 0) return "text/csv";
    if (strcmp(ext, ".gif") == 0) return "image/gif";
    if (strcmp(ext, ".png") == 0) return "image/png";
    if (strcmp(ext, ".css") == 0) return "text/css";
    if (strcmp(ext, ".au") == 0) return "audio/basic";
    if (strcmp(ext, ".wav") == 0) return "audio/wav";
    if (strcmp(ext, ".avi") == 0) return "video/x-msvideo";
    if (strcmp(ext, ".mpeg") == 0 || strcmp(ext, ".mpg") == 0) return "video/mpeg";
    if (strcmp(ext, ".mp3") == 0) return "audio/mpeg";
    
    #ifdef CGI
    if (strcmp(ext, ".cgi") == 0) return "cgi";
    #else
    if (strcmp(ext, ".cgi") == 0) return "text/plain";
    #endif

    return DEFAULT_MIMETYPE;
}


/*void send_file(char* path, struct stat *statbuf, char* vpath)
{
    char data[4096];
    int n;
    char* mime_type = get_mime_type(path);

    FILE *file = fopen(path, "r");
    if (!file) 
    {
        send_error(403, "Forbidden", NULL, "Access denied.");
        return;
    }

    // Grab the file length
    int length = S_ISREG(statbuf->st_mode) ? statbuf->st_size : -1;

    // Log the file request (CGI handles the log itself)
    #ifdef ONELOG
        logrequest(200, length);
    #endif

    // Send the file
    send_headers( 200, "OK", NULL, mime_type, length, statbuf->st_mtime, 1);

    while ((n = fread(data, 1, sizeof(data), file)) > 0) fwrite(data, 1, n, stdout);
    fclose(file);
}
*/

/**
 * Unescape URL encoded stuff
 */
void unescape(char* str, char* output) {

    char* c = str;
    char* o = output;
    char* end = (str + strlen(str));
    while (c < end) {
        if (strncmp(c, "%20", 3) == 0) {
            *o = ' '; c++; c++; o++;
        }
        else if (strncmp(c, "%24", 3) == 0) {
            *o = '$'; c++; c++; o++;
        }
        else if (strncmp(c, "%26", 3) == 0) {
            *o = '&'; c++; c++; o++;
        }
        else if (strncmp(c, "%2B", 3) == 0) {
            *o = '+'; c++; c++; o++;
        }
        else if (strncmp(c, "%2C", 3) == 0) {
            *o = ','; c++; c++; o++;
        }
        else if (strncmp(c, "%2F", 3) == 0) {
            *o = '/'; c++; c++; o++;
        }
        else if (strncmp(c, "%3A", 3) == 0) {
            *o = ':'; c++; c++; o++;
        }
        else if (strncmp(c, "%3B", 3) == 0) {
            *o = ';'; c++; c++; o++;
        }
        else if (strncmp(c, "%3D", 3) == 0) {
            *o = '='; c++; c++; o++;
        }
        else if (strncmp(c, "%3F", 3) == 0) {
            *o = '?'; c++; c++; o++;
        }
        else if (strncmp(c, "%40", 3) == 0) {
            *o = '@'; c++; c++; o++;
        }
        else {
            *o = *c;
            o++;
        }
        c++;
    }
    *o = '\0';
}

int process()
{
    char buf[TOKEN_SIZE];
    char* method;
    char* path;
    char* protocol;
    char* querystring;
    char* subpath;    
    char* ext;        
    char pathbuf[TOKEN_SIZE];
    char thepath[TOKEN_SIZE];
    char realpath[TOKEN_SIZE];
    struct stat statbuf;
    int len, i;
    int ispost = 0;

    if (!fgets(buf, sizeof(buf), stdin)) return -1;

    // Copy the request before we muck about with it and
    // remove the eof
    strcpy(request, buf);
    for (i = strlen(request); i > 0; i--) {
        if ( *(request + i) == '\r' ) {
            *(request +i) = '\0';
            break;
        }
    }

    // Get the individual bits of the request
    method = strtok(buf, " ");
    path = strtok(NULL, " ");
    protocol = strtok(NULL, "\r");

    printf("%s\r\n", method);    
    printf("%s\r\n", path);    
    printf("%s\r\n", protocol);                  
   
    // Stop if the request is incomplete
    if (!method || !path || !protocol) return -1;

    // Parse querystring if there is one
    querystring = strstr(path, "?");
    if (querystring)
        querystring += sizeof(char);

    
    printf("querystring: '%s'\n", querystring);

    subpath = strtok(path, "?");
    printf("subpath: '%s'\n", subpath);

    ext = strrchr(subpath, '.');
    printf ("ext: '%s'\n", ext);
    

    // Use the documentbase to find the full path to the document    
    //snprintf(thepath, sizeof(thepath), "%s%s", "/var/www", path);

    // Unescape any URL encoding
    //unescape(thepath, realpath);

    
    // Turn the querystring ? into a string terminator in the
    // file path since we aren't interested in it to find the file
    char* qsloc = strstr(realpath, "?");
    if (qsloc)
        *(qsloc) = '\0';

    // Read the HTTP headers and turn them into environment variables
    read_http_headers();
    
    // Flag posts so we know to redirect input when grabbing CGIs
    //if (strcmp(method, "POST") == 0) ispost = 1;

    // We only support GET and POST requests
    /*if (strcmp(method, "GET") != 0 && strcmp(method, "POST") != 0)
        send_error(501, "Not supported", NULL, "Method is not supported.");
    
    #ifdef NORELATIVE
    // Send forbidden for relative paths if we're stripping
    else if (strstr(realpath, ".."))
        send_error(403, "Forbidden", NULL, "Permission denied.");
    #endif

    // Drop out if the path is invalid
    else if (stat(realpath, &statbuf) < 0)
        send_error(404, "Not Found", NULL, "File not found.");

    // Path given is a directory
    else if (S_ISDIR(statbuf.st_mode))
    {
        len = strlen(realpath);
        // If it's a directory, redirect with a slash on the end
        if (len == 0 || realpath[len - 1] != '/')
        {
            snprintf(pathbuf, sizeof(pathbuf), "Location: %s/", path);
            send_error(302, "Found", pathbuf, "Directories must end with a slash.");
        }
        else
        {
            // If the directory has an index page, send that
            snprintf(pathbuf, sizeof(pathbuf), "%s%sindex.html", documentbase, path);
            if (stat(pathbuf, &statbuf) >= 0)
                send_file(pathbuf, &statbuf, NULL, NULL, 0);

            // Otherwise, generate a listing page and send that
            else
            {
                DIR *dir;
                struct dirent *de;

                send_headers(200, "OK", NULL, "text/html", -1, statbuf.st_mtime, 1);
                printf("<HTML><HEAD><TITLE>Index of %s</TITLE></HEAD>\r\n<BODY>", path);
                printf("<H4>Index of %s</H4>\r\n<PRE>\n", path);
                printf("Name Last Modified Size\r\n");
                printf("<HR>\r\n");
                if (len > 1) printf("<A HREF=\"..\">..</A>\r\n");

                dir = opendir(realpath);
                while ((de = readdir(dir)) != NULL)
                {
                    char timebuf[32];
                    struct tm *tm;

                    strcpy(pathbuf, realpath);
                    strcat(pathbuf, de->d_name);

                    stat(pathbuf, &statbuf);
                    tm = gmtime(&statbuf.st_mtime);
                    strftime(timebuf, sizeof(timebuf), "%d-%b-%Y %H:%M:%S", tm);

                    printf("<A HREF=\"%s%s\">", de->d_name, S_ISDIR(statbuf.st_mode) ? "/" : "");
                    printf("%s%s", de->d_name, S_ISDIR(statbuf.st_mode) ? "/</A>" : "</A> ");
                    if (strlen(de->d_name) < 32) printf("%*s", 32 - strlen(de->d_name), "");

                    if (S_ISDIR(statbuf.st_mode))
                        printf("%s\r\n", timebuf);
                    else
                        printf("%s %10d\r\n", timebuf, statbuf.st_size);
                }
                closedir(dir);

                printf("</PRE>\r\n<HR>\r\n<ADDRESS>%s</ADDRESS>\r\n</BODY></HTML>\r\n", SERVER);
            }
        }
    }    
    */
    //else
    //{
    //    send_file(realpath, &statbuf, path, querystring, ispost);
    //}
    //send_file(realpath, &statbuf, path);    
    return 0;
}

int main(int argc, char* argv[]) 
{
    process();
    //fwrite ( gifpixel, 1, (sizeof gifpixel) - 1, stdout );
    return 0;
}
