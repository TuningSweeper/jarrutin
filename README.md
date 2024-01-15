# jarrutin
 Sähköinen jarrutinprojekti
 
 Tästä himmelistä tulee isona sähköinen jarru erääseen muutoin vähemmän sähköiseen järjestelmään.




## Toimintalogiikasta

Jarrutus viimekädessä toimii oikosulkemalla kestomagneettimoottoria.

Moottori on kytketty kolmivaiheiseen diodisiltaan, minkä tarvittaessa oikosulkee n-fetti. Jännitteet ja virrat saattavat olla aika kovia. Sen takia fettiä ajetaan erillisellä ajurilla, mikä pitää huolen siitä, että tilanmuutokset ovat riittävän nopeita. Diodisillalla saadaan myös tasasuunnattua ja reguloitua muulle elektroniikalle käyttösähkö. Muutaman sadan mikrofaradin elko 7805:n tulopuolella jaksaa pitää prossun hengissä ehkä noin 100 ms.

Nopeutta haistelee optinen heijastusanturi, minkä kyljessä oleva CMOS-logiikka (4069, hex inverter) toimii ihan vain "matalan impedanssin" ajurina "pitkälle signaalitielle". Korkeaimpedanssinen signaalitie saattaisi muuten ottaa moottorin kentistä häiriöitä, mitkä virheellisesti tulkittaisiin nopeudeksi. Komponentin valintaperusteena on se, että se löytyi miljoonalaatikosta ensin. 40106 (Schmitt-trigger cmos hex inverter) voisi olla vielä vähän mukavampi. Optisessa anturissa voisi myös olla 10 Kohm herkkyydensäätöruuvi.

Raspi Zeron softalogiikka on yksinkertainen. Saapuvien pulssien määrää lasketaan keskeytyksellä. Tässä ohessa on jonkinverran filtteröintiä: a) yksittäiset pulssit vaativat, että signaalin tila ei muutu "pieneen hetkeen", että pulssi lasketaan ja b) pulssista pitää olla kulunut tietty minimiaika, että se lasketaan. Säännöllisin välein (esim. 8hz) tarkastetaan pulssien määrä ja pyöräytetään pulssimäärät eksponentiaalinen liukuvan keskiarvon läpi. Saatua lopputulosta verrataan asetusarvoihin, mikä asettaa PWM-lähdön pulssisuhteen. 100% ei voida koskaan saavuttaa, koska silloinhan prosessori sammuisi 100 ms päästä. Maksimi jarrutusteho onkin n. 98%




## Ajatuksia tulevaisuutta varten

Prosessoriksi PSoC1, jotta se pulssilaskurin toteutus olisi elegantti - kaikki palikat uC:n sisäänrakennetulla raudalla:

* Anturin LEDiä voisi ajaa kanttiaallolla
* Fotodiodin signaalin voisi tuoda sisään vahvistimen kautta
* Kaistanpäästösuotimella voisi ottaa vain anturin taajuuden
* Siihen perään verhokäyräilmaisu ja vahvistus
* Ja tämä pulssilaskurille.
* Olis hieno :)
