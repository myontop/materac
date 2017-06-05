#include "HX711.h"


#include "Arduino.h"
#include "Encoder.h"

class RunningAverage
{
public:
    RunningAverage(void);
    RunningAverage(int);
    ~RunningAverage();

    void clear();
    void addValue(double);
    void fillValue(double, int);

    double getAverage();

    double getElement(uint8_t idx);
    uint8_t getSize() { return _size; }
    uint8_t getCount() { return _cnt; }

protected:
    uint8_t _size;
    uint8_t _cnt;
    uint8_t _idx;
    double   _sum;
    double * _ar;
};

RunningAverage::RunningAverage(int n)
{
    _size = n;
    _ar = (double*) malloc(_size * sizeof(double));
    if (_ar == NULL) _size = 0;
    clear();
}

RunningAverage::~RunningAverage()
{
    if (_ar != NULL) free(_ar);
}

// resets all counters
void RunningAverage::clear()
{
    _cnt = 0;
    _idx = 0;
    _sum = 0.0;
    for (int i = 0; i< _size; i++) _ar[i] = 0.0;  // needed to keep addValue simple
}

// adds a new value to the data-set
void RunningAverage::addValue(double f)
{
    if (_ar == NULL) return;
    _sum -= _ar[_idx];
    _ar[_idx] = f;
    _sum += _ar[_idx];
    _idx++;
    if (_idx == _size) _idx = 0;  // faster than %
    if (_cnt < _size) _cnt++;
}

// returns the average of the data-set added sofar
double RunningAverage::getAverage()
{
    if (_cnt == 0) return NAN;
    return _sum / (double)_cnt;
}

// returns the value of an element if exist, 0 otherwise
double RunningAverage::getElement(uint8_t idx)
{
    if (idx >=_cnt ) return NAN;
    return _ar[idx];
}

// fill the average with a value
// the param number determines how often value is added (weight)
// number should preferably be between 1 and size
void RunningAverage::fillValue(double value, int number)
{
    clear();
    for (int i = 0; i < number; i++)
    {
        addValue(value);
    }
}


uint8_t PIN_HX711_DOUT = 15;
uint8_t PIN_HX711_PD_SCK = 14;

HX711 g_scale(PIN_HX711_DOUT, PIN_HX711_PD_SCK);
Encoder myEnc(2, 3);

RunningAverage myRA(10);

void setup()
{
 Serial.begin(57600);

 g_scale.set_scale(1);
 g_scale.power_up();
  myRA.clear(); // explicitly start clean
}

char buffer[200] = {0};

int a=0;
long oldPosition  = -999;
void loop()
{



  double tens = g_scale.get_value(2)/1342.0;

  {
  unsigned long m = millis();

  myRA.addValue(tens*100);
    sprintf(buffer, "s1>{\"time\":%lu,\"tens\":%lu,\"offset\":%ld,\"ra\":%lu}",m,(unsigned long)(tens*100), myEnc.read(),(unsigned long)(myRA.getAverage()));



    Serial.println(buffer);
  }
}
