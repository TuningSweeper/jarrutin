/* 
	Jarru
	
	Yritetty kirjoittaa niin, että on luettavissa ja ymmärrettävissä.
	Ei siis mitään turhan hienoja juttuja tai mitään kirjastoja.

	Arduino IDE: 2.2.1
	Board manager url: https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
  Board: Raspberry Pi Pico
	´

changelog

	ver 0.1:
		Eka proto Arduinolla (joku ATMEGA)
		Periaate todettiin toimivaksi
		
	ver 0.2:
		Porttaus Raspberry Pi Picolle
		Kommentoitu
		Parametrit nätisti
		Lisätty suodin vähän pehmentämään jarrutuksen muuttumista
*/

// jos tämä on määritelty, tulosteleee USB-uarttiin miten menee.
// tuotantoversiota varten kommentoi pois.
#define SERIALDEBUG

// brakeMinLevel kertoo millä nopeudella (pulssia/sekunti) jarrutus alkaa
#define brakeMinLevel 50

// brakeMaxLevel kertoo millä nopeudella (pulssia/sekunti) jarrutus on TÄYSILLÄ
#define brakeMaxLevel 400

// loopInterval (ms) kertoo kuinka usein tila päivitty. 125 ms, eli 8hz lienee aika sopiva
#define loopInterval 125

// nää määrittelis minne PWM ja sensori olis kytketty.
#define pwmOutputPin 5
#define sensorInputPin 13

// tämä kertoo käytetäänkö pehmentämiseen filtteriä vai ei. Kommentoi pois, jos ei.
#define useInputFilter
// Filtteri on eksponentiaalinen liukuva keskiarvo, eli viimeisintä arvoa painotetaan eniten ja vanhinta vähiten
// inputFilterN kertoo suotimen pisteiden määrän.  125 ms loopIntervallilla n=16 tarkoittaa kahta sekuntia
#define inputFilterN 12
// inputFilterAlpha on ulostulon suotimen painotusten "jyrkkyys".
#define inputFilterAlpha 0.2

// pwmFrequency kertoo millä taajuudella fettiä ajetaan. korkeampi taajuus vie vinkumisen korkeille taajuuksille (eikä kuulu, mutta fetti voi lämmetä enemmän)
#define pwmFrequency 8000

// pwmMax kertoo pwm-jarrun suurimman arvon. Sinne pitää jättää vähän löysää, että prossu pysyy virroissa. 255 arvo johtaa prossun boottaamiseen.
// arvolla 250 noin 2% moottorin tuottamasta energiasta on jarruohjaimen käytettävissä. Sen pitäis riittää.
#define pwmMax 250




/**********************************************************
  KOODI ALKAIS SIT TÄSTÄ
**********************************************************/ 

// ********
// filtteri
// ********

// Tämä taulukko hilloaa inputFilterN kpl edellistä sensorin raaka-arvoa.
unsigned int inputBuffer[inputFilterN];

// Tälle annetaan aina uusi "raaka-arvo", ja se pullauttaa ulos suodatetun arvon.
// suodatetun arvon perusteella voi sitten katsoa tarvitaanko jarrua vai ei, ja kuinka paljon.
// Tyypiltään tämä on eksponentiaalinen liukuva keskiarvo, eli se painottaa tuoreita arvoja enemmän kuin vanhoja arvoja.
unsigned int getFilteredInputValue(unsigned int value) {

	// siirrä arvoja yksi pykälä eteenpäin
	for(int i=inputFilterN-1; i>0; i--) {
		inputBuffer[i] = inputBuffer[i-1];
	}
	// ja laita uusin arvo indeksiin 0
	inputBuffer[0] = value;
	
	// ja filtteröi
	float filteredValue = inputBuffer[0];
  for (int i=0; i<inputFilterN; i++) {
      filteredValue = (inputFilterAlpha * inputBuffer[i]) + ((1.0 - inputFilterAlpha) * filteredValue);
  }

	// palautetaan se kokonaislukuna
	return (unsigned int) filteredValue;
}



// ********
// pulssilaskuri
// ********

volatile unsigned int pulseCount;
void pulseCounter() {
	// tää vain laskee saapuneita keskeytyksiä 
	pulseCount++;
}



// ********
// päälogiikka
// ********
unsigned long previousMillis = 0;
void loop() {
	unsigned int value, rawValue, pwmValue;
	
		unsigned long currentMillis = millis();
	if (currentMillis - previousMillis >= loopInterval) {
		previousMillis = currentMillis;
		
		// nollataan watchdog-ajastin
		rp2040.wdt_reset();
		
		// haetaan pulssien määrä
		rawValue = pulseCount;
		pulseCount = 0;
		
		// skaalataan saatu pulssimäärä taajuudeksi
		rawValue = rawValue * (1000/loopInterval);
		
		// tarviiko filtteröidä?
		#if defined(useInputFilter)
			value = getFilteredInputValue(rawValue);
		#else
			value = rawValue;
		#endif
		
		// funtsitaan jarrun asento.
		pwmValue = 0; // oletuksena jarru pois
		if(value > brakeMinLevel) {
			// tarvitaan jarrua, mutta kuinka paljon?
			if(value > brakeMaxLevel) {
				// täys jarru+
				pwmValue = pwmMax;
			} else {
				// sopiva jarru
				pwmValue = pwmMax * 1.0 / (brakeMaxLevel - brakeMinLevel) * (value - brakeMinLevel);
			}
		}		

		// asetetaan se PWM.
		analogWrite(pwmOutputPin, pwmValue);

		#if defined(SERIALDEBUG)
			Serial.print("raw value: ");
			Serial.print(rawValue);
			Serial.print("\t filtered value: ");
			Serial.print(value);
			Serial.print("\t PWM Setting: ");
			Serial.println(pwmValue);
		#endif
	}
}




// ********
// alustukset
// ********

void setup() {
	// Watchdog boottaa tän, jos käy hassusti.
	rp2040.wdt_begin(loopInterval*3);

	// Nollaillaan filtterin puskuri, jos tarvii.
	#if defined(useInputFilter)
		for(int i=0; i<inputFilterN; i++) inputBuffer[i] = 0;
	#endif

	// nollataan pulssilaskuri
	pulseCount = 0;

	// Tähän tulee nopeusanturin pulssit
	pinMode(sensorInputPin, INPUT);
	attachInterrupt(digitalPinToInterrupt(sensorInputPin), pulseCounter, RISING);

	// PWM lähtö
	pinMode(pwmOutputPin, OUTPUT);
	analogWriteFreq(pwmFrequency);
	analogWriteRange(255);
	analogWriteResolution(8);
	analogWrite(pwmOutputPin, 0);
	
	// debuggi-tuloste sarjaporttiin.
	#if defined(SERIALDEBUG)
		Serial.begin(115200);
	#endif

	// nollataan vielä watchdog-ajastin
	rp2040.wdt_reset();
}