/* 
 *  Sample Head and histogram reporter
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <Time.h>
#include <math.h>
#define true 1
#define false 0
#define MVREF 2560
#define ADCOUNT 1024
#define VDIVRATIO 2.5/3.5
#define MVPERDB 33.5
#define DBOFFSET 25.0

const float MVPERCOUNT = (float)MVREF/(float)ADCOUNT;
const int INPUT_PIN = A3;
const int sample_ms = 10;
const int samples_per_frame = 20;
const int bucket_tops[] = {0,4,9,14,19}; // which items in the sorted list to report, top of bucket (quartile bounds values)
time_t last_wakeup;
time_t wakeup_interval = sample_ms;

struct sample_record {
  int sample_value;
  int next_index; // next higher or equal value
} sample_frame[samples_per_frame];

int low_sample_index;

void rank_sample( int record_index ) {
  if (sample_frame[record_index].sample_value < sample_frame[low_sample_index].sample_value) {
    // insert this sample as low
    sample_frame[record_index].next_index = low_sample_index;
    low_sample_index = record_index;
    // Serial.print("low=");
    // Serial.println(low_sample_index);
  }
  else {
    // walk through all of the samples in the frame by following next_index until the new sample is 
    // less than the sample "after" the current sample, then insert between current and next 
    int scan_index = low_sample_index;
    for ( int count = 0; count < record_index; count ++ ) {
      if (( sample_frame[record_index].sample_value < sample_frame[sample_frame[scan_index].next_index].sample_value ) 
           || ( count+1 == record_index)) {
        // insert the value "after" scan_index
        sample_frame[record_index].next_index = sample_frame[scan_index].next_index;
        sample_frame[scan_index].next_index = record_index;
        break;
      }
      else {
        scan_index = sample_frame[scan_index].next_index;
      }
    }
  }
}

void setup() {
  //analogReference(INTERNAL);
  Serial.begin(9600);
  Serial.println();
  last_wakeup = millis();
}

void loop() {

  low_sample_index = 0;

  // fill the frame buffer and rank each entry as it is sampled
  for( int record_index = 0; record_index < samples_per_frame; record_index++ ) {
    while (millis() - last_wakeup < wakeup_interval) {
    } // busy wait for the next wakeup interval to pass
    last_wakeup += wakeup_interval; // preserve the relative time of each wakeup
    sample_frame[record_index].sample_value = analogRead(INPUT_PIN);
    rank_sample(record_index); // insert the pointer to the current sample in value rank order
  }

  // find the percentile summary values
  int record_index = low_sample_index; // start at the lowest value frame
  for ( int count = 0; count < samples_per_frame; count++ ) {
    for ( int bucket = 0; bucket < (sizeof(bucket_tops)/sizeof(int)); bucket++) {
      if (count == bucket_tops[bucket]) {
        Serial.print( ((float)sample_frame[record_index].sample_value * MVPERCOUNT / (MVPERDB*VDIVRATIO)) + DBOFFSET ,1 );
        if ( bucket+1 < (sizeof(bucket_tops)/sizeof(int)) ) {
          Serial.print(",");
        }
      }
    }
    record_index = sample_frame[record_index].next_index; // follow the value rank order, last one doesn't get used
  }
  Serial.println();
  
}
