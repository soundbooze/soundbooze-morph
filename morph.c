#include "loris.h"

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

int main( void )
{
   #define BUFSZ (3*44100)

   double samples[ BUFSZ ]; /* clarinet is about 3 seconds */
   unsigned int N = 0;
   
   PartialList * clar = createPartialList();
   PartialList * flut = createPartialList();
   LinearEnvelope * reference = 0;
   LinearEnvelope * pitchenv = createLinearEnvelope();

   LinearEnvelope * morphenv = createLinearEnvelope();
   PartialList * mrph = createPartialList();   

   double flute_times[] = {0.4, 1.};
   double clar_times[] = {0.2, 1.};
   double tgt_times[] = {0.3, 1.2};
   
   importSdif( "clarinet.sdif", clar );
      
   /* channelize and distill */
   reference = createF0Estimate( clar, 350, 450, 0.01 );
   channelize( clar, reference, 1 );
   distill( clar );
   destroyLinearEnvelope( reference );
   reference = 0;
      
   /* shift pitch of clarinet partials */
   printf( "shifting pitch of clarinet partials down by 600 cents\n" );
   linearEnvelope_insertBreakpoint( pitchenv, 0, -600 );
   shiftPitch( clar, pitchenv );
   destroyLinearEnvelope( pitchenv );
   pitchenv = 0;
   
   /* import the raw flute partials */
   printf( "importing flute partials\n" );
   importSdif( "flute.sdif", flut );
   
   /* channelize and distill */
   printf( "distilling\n" );
   reference = createF0Estimate( flut, 250, 320, 0.01 );
   channelize( flut, reference, 1 );
   distill( flut );
   destroyLinearEnvelope( reference );
   reference = 0;

   /* align onsets */
   printf( "dilating sounds to align onsets\n" );
   dilate( clar, clar_times, tgt_times, 2 );
   dilate( flut, flute_times, tgt_times, 2 );
   
   /* perform morph */
   printf( "morphing clarinet with flute\n" );
   linearEnvelope_insertBreakpoint( morphenv, 0.6, 0 );
   linearEnvelope_insertBreakpoint( morphenv, 2, 1 );
   morph( clar, flut, morphenv, morphenv, morphenv, mrph );
   
   /* synthesize and export samples */
   printf( "synthesizing %lu morphed partials\n", partialList_size( mrph ) );
   N = synthesize( mrph, samples, BUFSZ, 44100 );
   exportAiff( "morph.aiff", samples, N, 44100, 16 );
   
   /* cleanup */
   destroyPartialList( mrph );
   destroyPartialList( clar );
   destroyPartialList( flut );
   destroyLinearEnvelope( morphenv );
   
   return 0;
}
