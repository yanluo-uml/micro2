// Fall 2017 - Micro 2
// Author Ioannis Smanis

// face detection  dummy function

#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <linux/types.h>
#include <unistd.h>

// facedetection related library
#include "opencv2/objdetect/objdetect.hpp"




int face_detection (Mat taken_picture)
{
       //1. dynamic array to store all the faces detected      
       //2. Declare your cascade classifier variable
       // 3.  load cascade classifier  -> use the provided haarcascade_frontalface_alt2.xml or haarcascade_frontalface_alt.xml file

       //4. or  haarcascade_frontalface_alt.xml
      
         //5. error-check if the classifier is loaded or it is empty 
  
      
         // 6. call detectMultiScale() function  setting the rigth aguments 
         //  References: https://docs.opencv.org/3.3.0/d7/d8b/tutorial_py_face_detection.html  
         //  
                     http://opencvexamples.blogspot.com/2013/10/face-detection-using-haar-cascade.html
   

      //7. save face detected result

      //8. return 0 if no faces detection
       
      
      //9. return 1 if a face detected 
}
