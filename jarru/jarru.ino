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

	ver 0.3:
		Kovaa taistelua: tabs vs spaces
		Lisätty sensorled

	ver 0.4:
		tabs vs spaces erä II.
		Viety PWM-lähtö takanurkkaan
		Anturituloon alasvetovastus, ettei kellu.
		Askarreltu kohinasuodinta tuloon

	ver 0.5:
		tabs vs spaces erä III.
		askarreltu uusi status led
		test-moodi jarrun voimakkuudelle
		parametrisoitu input-kohina-filtteri
*/



// Tää moodi on vain jarrutusvoiman testaukseen. Jos määritelty, niin tää
// asettaa jarruvoiman asteikolla 0..255 tiettyyn tasoon eikä tee *mitään muuta*
// Huom. jarruvoimalla 255 (tai lähellä sitä) voimaa ei riitä cpu:lle ja se kaatu
//#define TESTMODE_BRAKELEVEL 30

// Tää moodi on vain anturin asemoinnin testaukseen. Jos määritelty, niin tää
// replikoi anturin tilaa, eikä tee *mitään muuta*
//#define TESTMODE_LEDBLINKER


// jos tää on määritelty, niin tää vilkuttelee sitä levyllä olevaa lediä anturin tahdissa.
// Huom. tän vilkku on *puolet* hitaampaa kuin mitä anturi näkee. Tuotantoa varten pois päältä.
#define SENSORLED

// jos tämä on määritelty, tulosteleee USB-uarttiin miten menee.
// tuotantoversiota varten kommentoi pois.
//#define SERIALDEBUG

// BRAKE_LEVEL_MIN kertoo millä nopeudella (pulssia/sekunti) jarrutus alkaa
#define BRAKE_LEVEL_MIN 8

// BRAKE_LEVEL_MAX kertoo millä nopeudella (pulssia/sekunti) jarrutus on TÄYSILLÄ
#define BRAKE_LEVEL_MAX 20

// LOOP_INTERVAL (ms) kertoo kuinka usein tila päivitty. 125 ms, eli 8hz lienee aika sopiva
#define LOOP_INTERVAL 125

// INPUT_PULSE_HOLDOFF (ms) kertoo kuinka pitä aika kahden pulssin välillä pitää vähintään olla, että
// se lasketaan. Tämä samalla rajaa systeemin laskeman maksiminopeuden. 10 ms = 100 pulssia sekunti = 6000 rpm.
#define INPUT_PULSE_HOLDOFF 10


// nää määrittelis minne PWM ja sensori ja ledi olis kytketty.
#define PWM_OUTPUT_PIN 15
#define SENSOR_INPUT_PIN 28
#define LED_PIN 2


// Tää kertoo käytetäänkö PWM-lähdön pehmentämiseen filtteriä vai ei.
#define USE_OUTPUT_FILTER
// Output filter on ihan vaan liukuva keskiarvo tietyllä N:llä.
#define OUTPUT_FILTER_N 12 

// Nämä kaksi kertoo, että kuinka monta kertaa tulosignaalin tila mitataan keskeytyksen liipaisun jälkeen
// ja kuinka monta näistä pitää olla samoja, että se pulssi lasketaan. Tämä siis suodattaa pois tosi nopeat piikit.
#define INPUT_PULSE_RECHECKS 3
#define INPUT_PULSE_RECHECK_MIN 3

// tämä kertoo käytetäänkö tulon pehmentämiseen filtteriä vai ei. Kommentoi pois, jos ei.
#define USE_INPUT_FILTER
// Filtteri on eksponentiaalinen liukuva keskiarvo, eli viimeisintä arvoa painotetaan eniten ja vanhinta vähiten
// INPUT_FILTER_N kertoo suotimen pisteiden määrän.  125 ms loopIntervallilla n=16 tarkoittaa kahta sekuntia
//#define INPUT_FILTER_N 12
#define INPUT_FILTER_N 4
// INPUT_FILTER_ALPHA on ulostulon suotimen painotusten "jyrkkyys".
//#define INPUT_FILTER_ALPHA 0.3
#define INPUT_FILTER_ALPHA 1

// PWM_FREQUENCY kertoo millä taajuudella fettiä ajetaan. korkeampi taajuus vie vinkumisen korkeille taajuuksille (eikä kuulu, mutta fetti voi lämmetä enemmän)
#define PWM_FREQUENCY 8000

// PWM_MAX kertoo pwm-jarrun suurimman arvon. Sinne pitää jättää vähän löysää, että prossu pysyy virroissa. 255 arvo johtaa prossun boottaamiseen.
// arvolla 250 noin 2% moottorin tuottamasta energiasta on jarruohjaimen käytettävissä. Sen pitäis riittää.
#define PWM_MAX 250




/**********************************************************
  KOODI ALKAIS SIT TÄSTÄ
**********************************************************/ 

// ********
// filtterit
// ********

// Tämä taulukko hilloaa INPUT_FILTER_N kpl edellistä sensorin raaka-arvoa.
unsigned int inputBuffer[INPUT_FILTER_N];

// Tälle annetaan aina uusi "raaka-arvo", ja se pullauttaa ulos suodatetun arvon.
// suodatetun arvon perusteella voi sitten katsoa tarvitaanko jarrua vai ei, ja kuinka paljon.
// Tyypiltään tämä on eksponentiaalinen liukuva keskiarvo, eli se painottaa tuoreita arvoja enemmän kuin vanhoja arvoja.
unsigned int getFilteredInputValue(unsigned int value) {

	// siirrä arvoja yksi pykälä eteenpäin
	for(int i=INPUT_FILTER_N-1; i>0; i--) {
		inputBuffer[i] = inputBuffer[i-1];
	}
	// ja laita uusin arvo indeksiin 0
	inputBuffer[0] = value;
	
	// ja filtteröi
	float filteredValue = inputBuffer[0];
	for (int i=0; i<INPUT_FILTER_N; i++) {
		filteredValue = (INPUT_FILTER_ALPHA * inputBuffer[i]) + ((1.0 - INPUT_FILTER_ALPHA) * filteredValue);
	}

	// palautetaan se kokonaislukuna
	return (unsigned int) filteredValue;
}


// Tämä taulukko hilloaa OUTPUT_FILTER_N kpl edellistä sensorin raaka-arvoa.
unsigned int outputBuffer[OUTPUT_FILTER_N];

// Tälle annetaan aina uusi "raaka-arvo", ja se pullauttaa ulos suodatetun arvon.
// Tyypiltään tämä on liukuva keskiarvo
unsigned int getFilteredOutputValue(unsigned int value) {

	// siirrä arvoja yksi pykälä eteenpäin
	for(int i=OUTPUT_FILTER_N-1; i>0; i--) outputBuffer[i] = outputBuffer[i-1];

	// ja laita uusin arvo indeksiin 0
	outputBuffer[0] = value;
	
	// ja filtteröi
	float filteredValue = 0;
	for (int i=0; i<OUTPUT_FILTER_N; i++) filteredValue = outputBuffer[i] + filteredValue;
	filteredValue = filteredValue/OUTPUT_FILTER_N;

	// palautetaan se kokonaislukuna
	return (unsigned int) filteredValue;
}



// ********
// pulssilaskuri
// ********

volatile unsigned long previousSensorInputMillis = 0;


volatile unsigned int pulseCount;
void pulseCounter() {
	int counter = 0;

	unsigned long currentMillis = millis();

	// Keskeytyksen lyödessä tarkistetaan pinnin tila vielä kolmesti.
	for(int i=0; i<INPUT_PULSE_RECHECKS; i++) {
		if(!digitalRead(SENSOR_INPUT_PIN)) counter++;
	}
	// Kaikkien kolmen pitää olla samat, muuten kyseessä kohina, mistä ei välitetä.
	if (counter >= INPUT_PULSE_RECHECK_MIN) {
		// Edellisestä pulssista pitää olla kulunut vähintään NNN millisekuntia,. 
		if(currentMillis >= (previousSensorInputMillis + INPUT_PULSE_HOLDOFF)) {
			pulseCount++;
		}
		previousSensorInputMillis = currentMillis;
	}

	// tää vähän auttaa sen anturin asemoinnissa.
	#if defined(SENSORLED)
		digitalWrite(LED_PIN, !digitalRead(LED_PIN));
	#endif
}




// ********
// päälogiikka
// ********
unsigned long previousMillis = 0;
void loop() {
	unsigned int value, rawValue, pwmValue;	
	unsigned long currentMillis = millis();

	if (currentMillis - previousMillis >= LOOP_INTERVAL) {
		previousMillis = currentMillis;
		
		// nollataan watchdog-ajastin
		rp2040.wdt_reset();
		
		// haetaan pulssien määrä
		rawValue = pulseCount;
		pulseCount = 0;
		
		// skaalataan saatu pulssimäärä taajuudeksi
		rawValue = rawValue * (1000/LOOP_INTERVAL);
		
		// tarviiko filtteröidä?
		#if defined(USE_INPUT_FILTER)
			value = getFilteredInputValue(rawValue);
		#else
			value = rawValue;
		#endif
		
		// funtsitaan jarrun asento.
		pwmValue = 0; // oletuksena jarru pois
		if(value > BRAKE_LEVEL_MIN) {
			// tarvitaan jarrua, mutta kuinka paljon?
			if(value > BRAKE_LEVEL_MAX) {
				// täys jarru+
				pwmValue = PWM_MAX;
			} else {
				// sopiva jarru
				pwmValue = PWM_MAX * 1.0 / (BRAKE_LEVEL_MAX - BRAKE_LEVEL_MIN) * (value - BRAKE_LEVEL_MIN);
			}
		}		

		// tarviiko filtteröidä?
		#if defined(USE_OUTPUT_FILTER)
			pwmValue = getFilteredOutputValue(pwmValue);
		#else
			pwmValue = pwmValue;
		#endif

		// asetetaan se PWM.
		analogWrite(PWM_OUTPUT_PIN, pwmValue);

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
	#if defined(SENSORLED)
		pinMode(LED_PIN, OUTPUT);
	#endif

	// Watchdog boottaa tän, jos käy hassusti.
	rp2040.wdt_begin(LOOP_INTERVAL*3);

	// Nollaillaan filtterin puskurit, jos tarvii.
	#if defined(useInputFilter)
		for(int i=0; i<INPUT_FILTER_N; i++) inputBuffer[i] = 0;
	#endif
	#if defined(useOutputFilter)
		for(int i=0; i<OUTPUT_FILTER_N; i++) outputBuffer[i] = 0;
	#endif

	// nollataan pulssilaskuri
	pulseCount = 0;

	// PWM lähtö
	pinMode(PWM_OUTPUT_PIN, OUTPUT);
	analogWriteFreq(PWM_FREQUENCY);
	analogWriteRange(255);
	analogWriteResolution(8);
	analogWrite(PWM_OUTPUT_PIN, 0);
	
	// Tää on testimoodi jarruvoiman testaukseen. Ei tuotantokäyttöön
	#if defined(TESTMODE_BRAKELEVEL)
		analogWrite(PWM_OUTPUT_PIN, TESTMODE_BRAKELEVEL);
		while(1) {
			digitalWrite(LED_PIN, !digitalRead(LED_PIN));
			delay(250);
			rp2040.wdt_reset();
		}
	#endif

	// Tää on testimoodi, joka replikoi sen anturin tilanteen. Ei tuotantoon
	// Huom. Tässä ei ole mitään kohinasuodinta mukana!
	#if defined(TESTMODE_LEDBLINKER)
	pinMode(SENSOR_INPUT_PIN, INPUT_PULLUP);
		while(1) {
			digitalWrite(LED_PIN, digitalRead(SENSOR_INPUT_PIN));
			rp2040.wdt_reset();
		}
	#endif

	// Tähän tulee nopeusanturin pulssit
	pinMode(SENSOR_INPUT_PIN, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(SENSOR_INPUT_PIN), pulseCounter, FALLING);

	// debuggi-tuloste sarjaporttiin, jos sellaista käytetään
	#if defined(SERIALDEBUG)
		Serial.begin(115200);
	#endif

	// nollataan vielä watchdog-ajastin
	rp2040.wdt_reset();
}