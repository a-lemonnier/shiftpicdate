# shiftpicdate

Shift the exif tag date using Exiv2 C++ library.

> Dependencies:
- C++ 17
- Boost
- Exiv2


```
Usage:
  -h [ --help ]            Display this help message.
  -q [ --quiet ]           toggle verbosity off.
  -f [ --filename ] arg    Specify one file.
  -D [ --directory ] arg   Directory where the pictures are.
  -t [ --thread ] arg (=4) Number of threads.
                           
  -S [ --second ] arg (=0)  
  -M [ --minute ] arg (=0)  
  -H [ --hour ] arg (=0)    
  -d [ --day ] arg (=0)     
  -m [ --month ] arg (=0)  Based on 30 days/month.
  -y [ --year ] arg (=0)   Based on 365 days/year.
  --DST                    Enable DST.

```
