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
		Anturituloon ylösvetovastus, ettei kellu.
		Askarreltu kohinasuodinta tuloon

	ver 0.5:
		tabs vs spaces erä III.
		askarreltu uusi status led
		test-moodi jarrun voimakkuudelle
		test-moodi anturin asemoinnille
		parametrisoitu input-kohina-filtteri
		uusi output filtteri

	ver 0.6:
		säädetty parametrejä. Tuntu jo ihan hyvältä

	ver 0.7:
		lisätty epälineaarisuus-optio. Paperilla se vaikuttaa hyvältä. EI OLE TESTATTU!
		Tuunattu parametrejä. Veikkaus on, että nyt on aika hyvä.
		Korjattu eksponentiaalisen liukuvan keskiarvon suotimen bugi
		Lisätty vähän kommentteja.
*/

// ****************
// Toimintamoodista

// Tää moodi on vain jarrutusvoiman testaukseen. Jos määritelty, niin tää
// asettaa jarruvoiman asteikolla 0..255 tiettyyn tasoon eikä tee *mitään muuta*
// Huom. jos laite ottaa oman virtansa moottorilta, niin jarruvoimalla 255
// (tai lähellä sitä) energiaa ei välttämätä riitä cpu:lle ja se kaatuu
//#define TESTMODE_BRAKELEVEL 250

// Tää moodi on vain anturin asemoinnin testaukseen. Jos määritelty, niin tää
// replikoi anturin tilaa, eikä tee *mitään muuta*. Eli vilkkuu kun pyörii.
//#define TESTMODE_LEDBLINKER

// jos tää on määritelty, niin tää vilkuttelee sitä levyllä olevaa lediä anturin tahdissa.
// Huom. tän vilkku on *puolet* hitaampaa kuin mitä anturi näkee. Tuotantoa varten pois päältä?
// Vie kuitenkin virtaa turhaan..
#define SENSORLED

// jos tämä on määritelty, tulosteleee USB-uarttiin miten menee.
// tuotantoversiota varten kommentoi pois.
// #define SERIALDEBUG


// ************************
// käytön asetuksia

// BRAKE_LEVEL_MIN kertoo millä nopeudella (pulssia/sekunti) jarrutus alkaa
//#define BRAKE_LEVEL_MIN 20
#define BRAKE_LEVEL_MIN 15

// BRAKE_LEVEL_MAX kertoo millä nopeudella (pulssia/sekunti) jarrutus on TÄYSILLÄ
//#define BRAKE_LEVEL_MAX 45
#define BRAKE_LEVEL_MAX 30

// LOOP_INTERVAL (ms) kertoo kuinka usein tila päivitty. 333 ms, eli 3hz lienee aika sopiva.
// Mitä tiheämmin systeemin tilaa päivitetään, sitä epätarkempi on nopeuden mittaus nykymallissa.
// Ja muutenkin: koska systeemissä on aika suuri liikemäärä, ei asioita tarvitse päivittää kovin
// ripeästi. Hihasta vedettynä 250 ms (eli 4hz) ... 500 ms (2 hz) kuulostaa sopivalta väliltä.
#define LOOP_INTERVAL 333


// **************************************
// optisen anturin kohinanpoistoa varten

// INPUT_PULSE_HOLDOFF (ms) kertoo kuinka pitkä aika kahden pulssin välillä pitää vähintään olla, että
// se lasketaan. Tämä samalla rajaa systeemin laskeman maksiminopeuden.
// esim. 10 ms = 100 pulssia sekunti. Jos kierros on 48 pulssia, niin se tarkoittaa 120 rpm.
// Arvolla 0 tämä optio on pois päältä.
#define INPUT_PULSE_HOLDOFF 0

// Nämä kaksi kertoo, että kuinka monta kertaa tulosignaalin tila mitataan keskeytyksen liipaisun jälkeen
// ja kuinka monta näistä pitää olla samoja, että se pulssi lasketaan. Tämä siis suodattaa pois tosi nopeat piikit.
// arvoilla 0 tämä feature on pois päältä.  Sen voi antaa olla jotain, niin eipähän pienet häiriöpiikit häiritse
// (vaikka ei niitä hall-anturn kohdalla varmaan nähdäkään)
#define INPUT_PULSE_RECHECKS 2
#define INPUT_PULSE_RECHECK_MIN 2



// **********************************************
// Raudan kytkennät ja pinnit ja siihen liittyvät

// nää määrittelis minne PWM ja sensori ja ledi olis kytketty.
#define PWM_OUTPUT_PIN 15
#define SENSOR_INPUT_PIN 28
#define LED_PIN 2

// PWM_FREQUENCY kertoo millä taajuudella fettiä ajetaan. korkeampi taajuus vie vinkumisen korkeille taajuuksille
// Korkeat taajuudet ei kuulu, mutta fetti voi lämmetä enemmän. Vois olla kiinnostavaa myös kokeilla
// pieniä arvoja miltä se *tuntuu* ja kuulostaa (100? en tiedä?).
#define PWM_FREQUENCY 2000

// PWM_MAX kertoo pwm-jarrun suurimman arvon välillä 0..255.
// JOS sen jarrun toimintasähkö otetaan moottorista, pitää tähän jättää vähän löysää, että prossun virtaa
// ei oteta kokonaan pois 100% pwm:llä. (255 arvo johtaa prossun boottaamiseen.)
// arvolla 251 noin 1% moottorin tuottamasta energiasta on jarruohjaimen käytettävissä.
#define PWM_MAX 255


// ***********************************************
// Filtterioptiot ja muut pehmentimet.

// USE_OUTPUT_FILTER kertoo käytetäänkö PWM-lähdön pehmentämiseen filtteriä vai ei.
// Kommentoi pois jos ei. Filtterin tyyppi on tavallinen liukuva keskiarvo.
#define USE_OUTPUT_FILTER
// Output filter on ihan vaan liukuva keskiarvo tietyllä N:llä. Jos LOOP_INTERVAL on 333 ja N on 6
// tarkoittaa se, sitä, että mutokset jarruvoimassa tulevat voimaan täysimääräisinä kahdessa sekunnissa.
#define OUTPUT_FILTER_N 6

// USE_INPUT_FILTER pehmentää myös jarruvoimaa, mutta tämä pehmentää anturilta tulevia lukemia.
//#define USE_INPUT_FILTER
// Filtteri on eksponentiaalinen liukuva keskiarvo, eli viimeisintä arvoa painotetaan eniten ja vanhinta vähiten
// INPUT_FILTER_N kertoo suotimen pisteiden määrän.  333 ms loopIntervallilla n=6 tarkoittaa kahta sekuntia
#define INPUT_FILTER_N 6
// INPUT_FILTER_ALPHA on ulostulon suotimen painotusten "jyrkkyys". Suurempi arvo painottaa tuoreita
// arvoja paljon enemmän. Arvolla 0 ei oikeastaan enää tehdä mitään filtteröintiä. 
#define INPUT_FILTER_ALPHA 0.4


// ********************************************************************
// Epälinearisointi

// Käytetäänkö jarrutustason asetuksessa epälineaarista funktiota?
// vaihtoehtoisinen optio on toistaiseksi POWER. ehkä joskus tulee toinenkin optio
// jos tämä on kommentoitu pois, niin PWM:n arvot ovat lineaariset.
#define NONLINEARIZATION_FUNCTION POWER

// Määrittää POWER -funktion eksponentin. Vaikuttaa sen "jyrkkyyteen". Pienempi on "jyrkempi"
// Kyseessä on yksinkertainen potenssifunktio. Millään tavalla järkeviä eksponentin arvoja
// on luvut välillä 0.1 .. 0.8.  joku 0.25 on ehkä hyvä aloituspiste.
// Funktion käppyrää voi tarkastella ja eksponenttiä tuunailla esim. URLissa:
// https://www.wolframalpha.com/input?i=plot+y%3D255*%28x%2F255%29%5E0.25+from+0+to+255
#define NONLINEARIZATION_EXPONENT 0.25



/**********************************************************
  KOODI ALKAIS SIT TÄSTÄ
**********************************************************/ 
#include <math.h>

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
	
	// ja filtteröi. Tässä siis lasketaan tietyllä tavalla painotettu summa puskurin arvoista
	// Tämä painottaa tuoreita arvoja enemmän kun vanhoja. 
	float filteredValue = inputBuffer[0];
	for (int i=INPUT_FILTER_N; i>0; i--) {
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



// Potenssifunktioskaalaus, eli tämä funktio tekee epälinearisoinnin
// yhtälö:  result = 255 * (input/255)^eksponentti
int powerScaling(int input) {
    double result;
    
    result = 255 * pow((double) input / 255, NONLINEARIZATION_EXPONENT);

    // Rajoita tulos välille 0-255
    if (result < 0) result = 0;
    if (result > 255) result = 255;

    return (int) result;
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
	unsigned int value, rawValue, rawPwmValue, pwmValue, pwmValueToSet;
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
		rawPwmValue = 0; // oletuksena jarru pois
		if(value > BRAKE_LEVEL_MIN) {
			// tarvitaan jarrua, mutta kuinka paljon?
			if(value > BRAKE_LEVEL_MAX) {
				// täys jarru+
				rawPwmValue = PWM_MAX;
			} else {
				// sopiva jarru
				rawPwmValue = PWM_MAX * 1.0 / (BRAKE_LEVEL_MAX - BRAKE_LEVEL_MIN) * (value - BRAKE_LEVEL_MIN);
			}
		}		

		// tarviiko filtteröidä?
		#if defined(USE_OUTPUT_FILTER)
			pwmValue = getFilteredOutputValue(rawPwmValue);
		#else
			pwmValue = rawPwmValue;
		#endif


		// Tarviiko se PWM:n arvo vielä epälinearisoida?
		#if NONLINEARIZATION_FUNCTION == POWER
			pwmValueToSet = powerScaling(pwmValue);
		#else
			pwmValueToSet = pwmValue;
		#endif


		// asetetaan se PWM.
		analogWrite(PWM_OUTPUT_PIN, pwmValueToSet);


		#if defined(SERIALDEBUG)
			Serial.print("input value: ");
			Serial.print(rawValue);
			Serial.print("\t filtered: ");
			Serial.print(value);
			Serial.print("\t raw PWM: ");
			Serial.print(rawPwmValue);
			Serial.print("\t filtered PWM Setting: ");
			Serial.print(pwmValue);
			Serial.print("\t nonlinearized PWM Setting: ");
			Serial.println(pwmValueToSet);
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
			delay(50);
			rp2040.wdt_reset();
		}
	#endif

	// Tää on testimoodi, joka replikoi sen anturin tilanteen. Ei tuotantoon
	// Huom. Tässä ei ole mitään kohinasuodinta mukana!
	#if defined(TESTMODE_LEDBLINKER)
		pinMode(SENSOR_INPUT_PIN, INPUT_PULLUP);
		while(1) {
			digitalWrite(LED_PIN, !digitalRead(SENSOR_INPUT_PIN));
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