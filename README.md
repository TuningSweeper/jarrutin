# jarrutin
 Sähköinen jarrutinprojekti
 
 Tästä himmelistä tulee isona sähköinen jarru erääseen muutoin vähemmän sähköiseen järjestelmään.




## Toimintalogiikasta

Jarrutus viimekädessä toimii oikosulkemalla kestomagneettimoottoria.

Moottori on kytketty kolmivaiheiseen diodisiltaan, minkä tarvittaessa oikosulkee n-fetti. Jännitteet ja virrat saattavat olla aika kovia. Sen takia fettiä ajetaan erillisellä ajurilla, mikä pitää huolen siitä, että tilanmuutokset ovat riittävän nopeita. Viimeisimmässä protossa fettejä on neljä rinnan, ettei paketti kuumene liikaa. Seuraavaa iteraatioon pitäisi vähän parantaa diodisillan ja fettien jäähdytystä. Siinä ehjä kannattaa unohtaa piirilevyt ja laittaa reilut rivat, ja niihin komponentit kiinni.

Nopeusanturina on moottorin oma hall-anturi. Sille annetaan 5v ja sieltä tulee avokollektorilähdön kautta nopeustieto. 4N35 -optoerottimella saadaan tehtyä optinen erotus ja samalla jännitteenmuunnos 3.3 V logiikalle. On vähän epäselvää mikä se hall-anturi on, ja toimisiko se kolmella voltilla. Jokatapauksessa optoerotus ei tee pahaa.

Prosessorina on Raspi Zero, mitä voi koodata Arduino IDE:llä. Softalogiikka on yksinkertainen: saapuvien pulssien määrää lasketaan keskeytyksellä. Tässä ohessa on jonkinverran filtteröintiä: a) yksittäiset pulssit vaativat, että signaalin tila ei muutu "pieneen hetkeen", että pulssi lasketaan ja b) pulssista pitää olla kulunut tietty minimiaika, että se lasketaan. Säännöllisin välein (esim. 3hz) tarkastetaan pulssien määrä ja pyöräytetään pulssimäärät sopivan filtterin läpi. Saatua lopputulosta verrataan asetusarvoihin, mikä asettaa PWM-lähdön pulssisuhteen.  Asetusarvoja on kaksi: missä nopeudessa jarru menee päälle ja missä nopeudessa se on täysillä. Tässä välissä voidaan mennä lineaarisesti tai epälineaarisesti.


## Ajatuksia

Diodisillalla saadaan myös tarvittaessa tasasuunnattua ja reguloitua muulle elektroniikalle käyttösähkö. Muutaman sadan mikrofaradin elko tulopuolella jaksaa pitää prossun hengissä ehkä noin 100 ms. Tässä pitää huomioida, että tällöin PWM:n suurin pulssisuhde ei voi olla 100%. Systeemissä on kovat jännitteet, mutta kaupasta saanee sopivia regulaattoreita, millä diodillalta saadaan reguloitua 5V sähkö. fi.mouser.com varmasti auttaa asiassa.



