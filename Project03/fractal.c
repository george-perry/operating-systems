/*
fractal.c - Mandelbrot fractal generation
Starting code for CSE 30341 Project 3 - Spring 2023
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <complex.h>
#include <pthread.h>

#include "bitmap.h"
#include "fractal.h"

// Function to print out command line arguments
void print_help() {
    printf("Help Information:\n");
    printf("-help         Display the help information\n");
    printf("-xmin X       New value for x min\n");
    printf("-xmax X       New value for x max\n");
    printf("-ymin Y       New value for y min\n");
    printf("-ymax Y       New value for y max\n");
    printf("-maxiter N    New value for the maximum number of iterations (must be an integer)\n");
    printf("-width W      New width for the output image\n");
    printf("-height H     New height for the output image\n");
    printf("-output F     New name for the output file\n");
    printf("-threads N    Number of threads to use for processing (default is 1)\n");
    printf("-row          Run using a row-based approach\n");
    printf("-task         Run using a thread-based approach\n");
    exit(-1);
}

/*
Compute the number of iterations at point x, y
in the complex space, up to a maximum of maxiter.
Return the number of iterations at that point.

This example computes the Mandelbrot fractal:
z = z^2 + alpha

Where z is initially zero, and alpha is the location x + iy
in the complex plane.  Note that we are using the "complex"
numeric type in C, which has the special functions cabs()
and cpow() to compute the absolute values and powers of
complex values.
*/

static int compute_point( double x, double y, int max )
{
	double complex z = 0;
	double complex alpha = x + I*y;

	int iter = 0;

	while( cabs(z)<4 && iter < max ) {
		z = cpow(z,2) + alpha;
		iter++;
	}

	return iter;
}

/*
Compute an entire image, writing each point to the given bitmap.
Scale the image to the range (xmin-xmax,ymin-ymax).

HINT: Generally, you will want to leave this code alone and write your threaded code separately

*/

void compute_image_singlethread ( struct FractalSettings * pSettings, struct bitmap * pBitmap)
{
	int i,j;

	// For every pixel i,j, in the image...

	for(j=0; j<pSettings->nPixelHeight; j++) {
		for(i=0; i<pSettings->nPixelWidth; i++) {

			// Scale from pixels i,j to coordinates x,y
			double x = pSettings->fMinX + i*(pSettings->fMaxX - pSettings->fMinX) / pSettings->nPixelWidth;
			double y = pSettings->fMinY + j*(pSettings->fMaxY - pSettings->fMinY) / pSettings->nPixelHeight;

			// Compute the iterations at x,y
			int iter = compute_point(x,y,pSettings->nMaxIter);

			// Convert a iteration number to an RGB color.
			// (Change this bit to get more interesting colors.)
			int gray = 255 * iter / pSettings->nMaxIter;

            // Set the particular pixel to the specific value
			// Set the pixel in the bitmap.
			bitmap_set(pBitmap,i,j,gray);
		}
	}
}

// New function based off compute_image_singlethread for row mode

void *compute_image_thread_row(void *args)
{
    struct thread_args *targs = (struct thread_args *)args;
    int i, j;

    for (j = targs->start; j < targs->end; j++) {
        for (i = 0; i < targs->pSettings->nPixelWidth; i++) {

            double x = targs->pSettings->fMinX + i * (targs->pSettings->fMaxX - targs->pSettings->fMinX) / targs->pSettings->nPixelWidth;
            double y = targs->pSettings->fMinY + j * (targs->pSettings->fMaxY - targs->pSettings->fMinY) / targs->pSettings->nPixelHeight;

            int iter = compute_point(x, y, targs->pSettings->nMaxIter);

            int gray = 255 * iter / targs->pSettings->nMaxIter;

            bitmap_set(targs->pBitmap, i, j, gray);
        }
    }

    pthread_exit(NULL);
}

// Function used for row mode using multiple threads
void compute_image_multiplethread_row(struct FractalSettings *pSettings, struct bitmap *pBitmap)
{
    // Initialize specified # of threads
    pthread_t threads[pSettings->nThreads];
    int thread_args[pSettings->nThreads];
    int i, rc;

    // Calculate rows per thread
    int rows_per_thread = pSettings->nPixelHeight / pSettings->nThreads;

    // Save attributes into new struct and create threads
    for (i = 0; i < pSettings->nThreads; i++) {
        struct thread_args *targs = malloc(sizeof(struct thread_args));
        targs->start = i * rows_per_thread;
        targs->end = (i == pSettings->nThreads - 1) ? pSettings->nPixelHeight : (i + 1) * rows_per_thread;
        targs->pSettings = pSettings;
        targs->pBitmap = pBitmap;
        rc = pthread_create(&threads[i], NULL, compute_image_thread_row, (void *)targs);
        if (rc) {
            fprintf(stderr, "Error creating thread: %d\n", rc);
            exit(-1);
        }
    }

    // Join the threads
    for (i = 0; i < pSettings->nThreads; i++) {
        rc = pthread_join(threads[i], NULL);
        if (rc) {
            fprintf(stderr, "Error joining thread: %d\n", rc);
            exit(-1);
        }
    }
}

// New function based off compute_image_singlethread for task mode
void *compute_image_thread_task(void *args) {

    // Create new struct to save attributes
    struct thread_args *targs = (struct thread_args *)args;
    struct task *task;
    int i, j;

    // Make sure to protect critical data in look using locks
    while (1) {
        // Check if there are any remaining tasks
        pthread_mutex_lock(targs->pMutex);
        if (*(targs->n_tasks_remaining) == 0) {
            pthread_mutex_unlock(targs->pMutex);
            break;
        }
        // Get the next available task
        for (i = 0; i < targs->pSettings->nPixelHeight; i++) {
            if (targs->tasks[i] != NULL) {
                task = targs->tasks[i];
                targs->tasks[i] = NULL;
                (*(targs->n_tasks_remaining))--;
                break;
            }
        }
        pthread_mutex_unlock(targs->pMutex);

        // Compute the assigned task
        for (j = task->start_row; j < task->end_row; j++) {
            for (i = 0; i < targs->pSettings->nPixelWidth; i++) {
                double x = targs->pSettings->fMinX + i * (targs->pSettings->fMaxX - targs->pSettings->fMinX) / targs->pSettings->nPixelWidth;
                double y = targs->pSettings->fMinY + j * (targs->pSettings->fMaxY - targs->pSettings->fMinY) / targs->pSettings->nPixelHeight;
                int iter = compute_point(x, y, targs->pSettings->nMaxIter);
                int gray = 255 * iter / targs->pSettings->nMaxIter;
                bitmap_set(targs->pBitmap, i, j, gray);
            }
        }

        // Free the completed task
        free(task);
    }

    pthread_exit(NULL);
}

void compute_image_multitask(struct FractalSettings *pSettings, struct bitmap *pBitmap) {
    
    // Initialize specified # of threads
    pthread_t threads[pSettings->nThreads];
    pthread_mutex_t mutex;

    // Variable to store # of remaining tasks based off height
    int n_tasks_remaining = pSettings->nPixelHeight;

    // Create new task array
    struct task **tasks = malloc(pSettings->nPixelHeight * sizeof(struct task *));
    int i, rc;

    // Initialize the tasks array
    for (i = 0; i < pSettings->nPixelHeight; i++) {
        tasks[i] = malloc(sizeof(struct task));
        tasks[i]->start_row = i;
        tasks[i]->end_row = i + 1;
    }

    // Initialize the mutex
    pthread_mutex_init(&mutex, NULL);

    // Start the threads
    for (i = 0; i < pSettings->nThreads; i++) {
        struct thread_args *targs = malloc(sizeof(struct thread_args));
        targs->pSettings = pSettings;
        targs->pBitmap = pBitmap;
        targs->pMutex = &mutex;
        targs->tasks = tasks;
        targs->n_tasks_remaining = &n_tasks_remaining;
        rc = pthread_create(&threads[i], NULL, compute_image_thread_task, (void *)targs);
        if (rc) {
            fprintf(stderr, "Error creating thread: %d\n", rc);
            exit(-1);
        }
    }

    // Join threads
    for (i = 0; i < pSettings->nThreads; i++) {
        rc = pthread_join(threads[i], NULL);
        if (rc) {
            fprintf(stderr, "Error joining thread: %d\n", rc);
            exit(-1);
        }
    }
}

/* Process all of the arguments as provided as an input and appropriately modify the
   pSettings for the project 
   @returns 1 if successful, 0 if unsuccessful (bad arguments) */
char processArguments (int argc, char * argv[], struct FractalSettings * pSettings)
{
    /* If we don't process anything, it must be successful, right? */

    // Initialize variables for processing input
    int i;
    double d;
    char *endptr;
    const char *arg;
    const char *argval;

    // Loop through all args
    for (i = 1; i < argc; i++) {
        arg = argv[i];

        // If -help entered, print out help message
        if (strcmp(arg, "-help") == 0) {
            print_help();
        } 
        
        // For the rest of the arguments, make sure the input is valid, save to struct
        // If invalid, return 0, else return 1

        // For -xmin, -xmax, -ymin, etc. , use strtod to convert arg to float

        else if (strcmp(arg, "-xmin") == 0) {
            argval = argv[++i];
            d = strtod(argval, &endptr);

            if (*endptr != '\0') {
                printf("Invalid value for -xmin: %s\n", argval);
                return 0;
            }

            if (d > pSettings->fMaxX) {
                printf("-xmin cannot be greater than -xmax\n");
                return 0;
            }
            pSettings->fMinX = d;
        } 
        
        else if (strcmp(arg, "-xmax") == 0) {
            argval = argv[++i];
            d = strtod(argval, &endptr);
            if (*endptr != '\0') {
                printf("Invalid value for -xmax: %s\n", argval);
                return 0;
            }
            if (d < pSettings->fMinX) {
                printf("-xmax cannot be less than -xmin\n");
                return 0;
            }
            pSettings->fMaxX = d;
        } 
        
        else if (strcmp(arg, "-ymin") == 0) {
            argval = argv[++i];
            d = strtod(argval, &endptr);
            if (*endptr != '\0') {
                printf("Invalid value for -ymin: %s\n", argval);
                return 0;
            }
            if (d > pSettings->fMaxY) {
                printf("-ymin cannot be greater than -ymax\n");
                return 0;
            }
            pSettings->fMinY = d;
        } 
        
        else if (strcmp(arg, "-ymax") == 0) {
            argval = argv[++i];
            d = strtod(argval, &endptr);
            if (*endptr != '\0') {
                printf("Invalid value for -ymax: %s\n", argval);
                return 0;
            }
            if (d < pSettings->fMinY) {
                printf("-ymax cannot be less than -ymin\n");
                return 0;
            }
            pSettings->fMaxY = d;

        } 
        
        else if (strcmp(argv[i], "-maxiter") == 0) {
            i++;
            int n = atoi(argv[i]);
            if (n > 0) {
                pSettings->nMaxIter = n;
            } else {
                printf("Error: Invalid value for -maxiter argument\n");
                return 0;
            }
        } 
        
        else if (strcmp(argv[i], "-width") == 0) {
            i++;
            int w = atoi(argv[i]);
            if (w > 0 && w <= DEFAULT_PIXEL_WIDTH) {
                pSettings->nPixelWidth = w;
            } else {
                printf("Error: Invalid value for -width argument\n");
                return 0;
            }
        } 
        
        else if (strcmp(argv[i], "-height") == 0) {
            i++;
            int h = atoi(argv[i]);
            if (h > 0 && h <= DEFAULT_PIXEL_HEIGHT) {
                pSettings->nPixelHeight = h;
            } else {
                printf("Error: Invalid value for -height argument\n");
                return 0;
            }
        } 
        
        else if (strcmp(argv[i], "-output") == 0) {
            i++;
            char* outfile = argv[i];
            if (strlen(outfile) <= MAX_OUTFILE_NAME_LEN) {
                strcpy(pSettings->szOutfile, outfile);
            } else {
                printf("Error: Output filename is too long\n");
                return 0;
            }
        } 
        
        else if (strcmp(argv[i], "-threads") == 0) {
            i++;
            int t = atoi(argv[i]);
            if (t > 0) {
                pSettings->nThreads = t;
            } else {
                printf("Error: Invalid value for -threads argument\n");
                return 0;
            }
        } 
        
        // For inputs -row and -task, ensure that both are not entered

        else if (strcmp(argv[i], "-row") == 0) {
            if (pSettings->theMode != MODE_THREAD_SINGLE) {
                printf("Error - Cannot Use Task and Row Mode\n");
                return 0;

            }
            pSettings->theMode = MODE_THREAD_ROW;
        } 
        
        else if (strcmp(argv[i], "-task") == 0) {
            if (pSettings->theMode != MODE_THREAD_SINGLE) {
                printf("Error - Cannot Use Task and Row Mode\n");
                return 0;

            }
            pSettings->theMode = MODE_THREAD_TASK;
        } 
        
        // Print error if unsuccessful
        else {
            printf("Error: Invalid argument\n");
            return 0;
        }
    }

    return 1;
}

int main( int argc, char *argv[] )
{
    struct FractalSettings  theSettings;

	// The initial boundaries of the fractal image in x,y space.
    theSettings.fMinX = DEFAULT_MIN_X;
    theSettings.fMaxX = DEFAULT_MAX_X;
    theSettings.fMinY = DEFAULT_MIN_Y;
    theSettings.fMaxY = DEFAULT_MAX_Y;
    theSettings.nMaxIter = DEFAULT_MAX_ITER;

    theSettings.nPixelWidth = DEFAULT_PIXEL_WIDTH;
    theSettings.nPixelHeight = DEFAULT_PIXEL_HEIGHT;

    theSettings.nThreads = DEFAULT_THREADS;
    theSettings.theMode  = MODE_THREAD_SINGLE;
    
    strncpy(theSettings.szOutfile, DEFAULT_OUTPUT_FILE, MAX_OUTFILE_NAME_LEN);

    /* TODO: Adapt your code to use arguments where the arguments can be used to override 
             the default values 

        -help         Display the help information
        -xmin X       New value for x min
        -xmax X       New value for x max
        -ymin Y       New value for y min
        -ymax Y       New value for y max
        -maxiter N    New value for the maximum number of iterations (must be an integer)     
        -width W      New width for the output image
        -height H     New height for the output image
        -output F     New name for the output file
        -threads N    Number of threads to use for processing (default is 1) 
        -row          Run using a row-based approach        
        -task         Run using a thread-based approach

        Support for setting the number of threads is optional

        You may also appropriately apply reasonable minimum / maximum values (e.g. minimum image width, etc.)
    */


   /* Are there any locks to set up? */

   if(processArguments(argc, argv, &theSettings))
   {
        /* Dispatch here based on what mode we might be in */
        if(theSettings.theMode == MODE_THREAD_SINGLE)
        {
            /* Create a bitmap of the appropriate size */
            struct bitmap * pBitmap = bitmap_create(theSettings.nPixelWidth, theSettings.nPixelHeight);

            /* Fill the bitmap with dark blue */
            bitmap_reset(pBitmap,MAKE_RGBA(0,0,255,0));

            /* Compute the image */
            compute_image_singlethread(&theSettings, pBitmap);
            printf("%s\n", theSettings.szOutfile);

            // Save the image in the stated file.
            if(!bitmap_save(pBitmap,theSettings.szOutfile)) {
                fprintf(stderr,"fractal: couldn't write to %s: %s\n",theSettings.szOutfile,strerror(errno));
                return 1;
            }            
        }
        else if(theSettings.theMode == MODE_THREAD_ROW)
        {
            /* A row-based approach will not require any concurrency protection */

            /* Could you send an argument and write a different version of compute_image that works off of a
               certain parameter setting for the rows to iterate upon? */

            /* Create a bitmap of the appropriate size */
            struct bitmap * pBitmap = bitmap_create(theSettings.nPixelWidth, theSettings.nPixelHeight);

            /* Fill the bitmap with dark blue */
            bitmap_reset(pBitmap,MAKE_RGBA(0,0,255,0));

            /* Compute the image */
            compute_image_multiplethread_row(&theSettings, pBitmap);

            // Save the image in the stated file.
            if(!bitmap_save(pBitmap,theSettings.szOutfile)) {
                fprintf(stderr,"fractal: couldn't write to %s: %s\n",theSettings.szOutfile,strerror(errno));
                return 1;
            }         

        }
        else if(theSettings.theMode == MODE_THREAD_TASK)
        {
            /* For the task-based model, you will want to create some sort of a way that captures the instructions
               or task (perhaps say a startX, startY and stopX, stopY in a struct).  You can have a global array 
               of the particular tasks with each thread attempting to pop off the next task.  Feel free to tinker 
               on what the right size of the work unit is but 20x20 is a good starting point.  You are also welcome
               to modify the pSettings struct to help you out as well.  
               
               Generally, it will be good to create all of the tasks into that array and then to start your threads
               with them in turn attempting to pull off a task one at a time.  
               
               While we could do condition variables, there is not really an ongoing producer if we create all of
               the tasks at the outset. Hence, it is OK whenever a thread needs something to do to try to access
               that shared data structure with all of the respective tasks.  
               */

            /* Create a bitmap of the appropriate size */
            struct bitmap * pBitmap = bitmap_create(theSettings.nPixelWidth, theSettings.nPixelHeight);

            /* Fill the bitmap with dark blue */
            bitmap_reset(pBitmap,MAKE_RGBA(0,0,255,0));

            /* Compute the image */
            compute_image_multitask(&theSettings, pBitmap);

            // Save the image in the stated file.
            if(!bitmap_save(pBitmap,theSettings.szOutfile)) {
                fprintf(stderr,"fractal: couldn't write to %s: %s\n",theSettings.szOutfile,strerror(errno));
                return 1;
            }         

        }
        else 
        {
            /* Uh oh - how did we get here? */
            printf("Error: Invalid Arguments\n");
            exit(-1);
        }
   }
   else
   {
        /* Probably a great place to dump the help */
        print_help();

        /* Probably a good place to bail out */
        exit(-1);
   }

    /* TODO: Do any cleanup as required */

	return 0;
}
