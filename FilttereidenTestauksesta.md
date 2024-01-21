# Filtteriden tuunauksesta

Systeemissä on pari jännempää filtteriä: on eksponentiaalinen liukuva keskiarvo ja sitten on epälinaarisuutta tuomassa potenssifunktio. Näitä voi testata ja ihmetellä kaikkein helpoiten erilaisilla www-pohjaisilla c-kääntäjillä.

Esim. sivustoon https://www.onlinegdb.com/online_c_compiler voi pasteta c-koodin ja painaa run, minkä jälkeen näkee mitä koodi tekee. Tuolle voi siis tarjota seuraavia testipalikoita. Se toivottavasti auttaa ymmärtämään miten ne toimii.


## Eksponentiaalinen liukuva keskiarvo

```
#include <stdio.h>

#define INPUT_FILTER_N 6
#define INPUT_FILTER_ALPHA 0.4

unsigned int inputBuffer[INPUT_FILTER_N];

unsigned int getFilteredInputValue(unsigned int value) {
	// siirrä arvoja yksi pykälä eteenpäin
	for(int i=INPUT_FILTER_N-1; i>0; i--) {
		inputBuffer[i] = inputBuffer[i-1];
	}
	// ja laita uusin arvo indeksiin 0
	inputBuffer[0] = value;
	
	// ja filtteröi. Tässä siis lasketaan tietyllä tavalla painotettu summa puskurin arvoista
	// ja siitä sitten 
	float filteredValue = inputBuffer[0];
	for (int i=INPUT_FILTER_N; i>0; i--) {
		filteredValue = (INPUT_FILTER_ALPHA * inputBuffer[i]) + ((1.0 - INPUT_FILTER_ALPHA) * filteredValue);
	}

	// palautetaan se kokonaislukuna
	return (unsigned int) filteredValue;
}

int main() {
    int val[20] = {0,0,0,0,0,0,10,10,10,10,20,10,10,10,10,10,10,10,10,10,10};
    // Write C code here
    for(int i=0; i<20; i++) {
        printf("in: %i   out: %i\n", val[i], getFilteredInputValue(val[i]));
    }
    return 0;
}
```



## Epälineaarisuutta tuova potenssifunktio
Funktion käppyrää voi tarkastella ja eksponenttiä tuunailla esim. URLissa:
https://www.wolframalpha.com/input?i=plot+y%3D255*%28x%2F255%29%5E0.25+from+0+to+255

```
#include <stdio.h>
#include <math.h>


double powerScaling(int input, double exponent) {
    double result;
    
    result = 255 * pow((double) input / 255, exponent);
    if (result < 0) result = 0;
    if (result > 255) result = 255;

    return result;
}

int main() {
    double exponent = 0.25;
    double result;
    

    for (int i = 0; i<256; i=i+1) {
        result = powerScaling(i, exponent);
        printf("input %i  \t   power scaling output: %i\n", i, (int) result);
    }

    return 0;
}
```
