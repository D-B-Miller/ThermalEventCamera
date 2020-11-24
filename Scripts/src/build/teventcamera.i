%module example
 %{
 /* Includes the header in the wrapper code */
 #include "teventcamera.h"
 %}
 
 /* Parse the header file to generate wrappers */
 %include "teventcamera.h"
