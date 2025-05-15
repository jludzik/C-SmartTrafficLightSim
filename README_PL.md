# Symulacja skrzyżowania z inteligentnymi światłami drogowymi

## Kompilacja
- Język: C (C18)
- Kompilator: x86_64-w64-mingw32
- Wersja: 14.1.0

W projekcie wykorzystano bibliotekę [cJSON](https://github.com/DaveGamble/cJSON)

Kompilacje można wykonać z poziomu terminala:

gcc -o build/main.exe src/main.c src/rw_logic.c src/traffic.c libs/cJSON/cJSON.c -Iinclude -Wall -std=c18

Lub za pomocą pliku makefile



## Uruchamianie symulacji

Aby uruchomić symulację, należy przekazać programowi **dwa argumenty**:

1. Ścieżka do pliku wejściowego `*.json` zawierającego polecenia
2. Ścieżka do pliku wyjściowego `*.json`

Przykładowy plik wejściowy znajduje się w katalogu: `example_json_files`



## Założenia symulacji
- W jednym kroku tylko jedna droga ma zielone światło
- W jednym kroku tylko jedno jedno auto przejeżdza przez skrzyżowanie jeśli:
  - ma zielone światło
  - jest pierwsze w kolejce
- Pojazdy muszą mieć unikalne identyfikatory w celu rozróżnienia ich podczas analizy pliku wyjściowego symulacji
- Identyfikatory pojazdów tworzone są według wzoru: `vehicleX`, gdzie X >= 0
- Po opuszczeniu skrzyżowania przez pojazd w jego strukturze `Car` w zmiennej int outSimStep zapisywany jest krok symulacji, w którym to nastąpiło
  - (-1) w programie oznacza, że pojazd nie opuścił skrzyżowania
- Stany świateł przechowywane są w strukturze `Road` w zmiennej typu `enum TrafficLightState`:
  - `LIGHT_RED` - czerwone - pojazdy stoją
  - `LIGHT_YELLOW_TO_RED` - żółte (następuje zmiana na czerwone) - pojazdy stoją
  - `LIGHT_YELLOW_TO_GREEN` - żółte (następuje zmiana na zielone) - pojazdy stoją
  - `LIGHT_GREEN` - zielone - pojazdy jadą
  


##  Organizacja i przetwarzanie danych

1. Po uruchomieniu programu:
   - Wczytywana jest liczba kroków do zmiennej `int simSteps`
   - Pojazdy trafiają do dynamicznej struktury danych **kolejki FIFO** wskazywanej przez `parkingHead`

2. W każdym kroku symulacji kolejno:
   - Sprawdzane jest, czy nowe pojazdy dojechały do skrzyżowania, jeśli tak to są one wyciągane z kolejki `parkingHead` i dodawane do jednej z czterech dróg. Każda droga również jest reprezentowana przez **kolejke FIFO**
    - Następnie jeśli jakiś samochód opuścił skrzyżowanie jest on wyjmowany z kolejki drogi i zapisywany w kolejce wyjściowej `outputHead` wraz z informacją w zmiennej `outSimStep` o kroku opuszczenia skrzyżowania

3. Po zakończeniu symulacji:
   - Dane z kolejki wyjściowej są zapisywane do pliku `*.json`



## Algorytm zmiany świateł

1. Zmiana świateł następuje według algorytmu opartego o ilość pojazdów na drodze i czas trwania czerwonego światła z różnymi wagami.

2. Obliczanie priorytetu
  - Jeśłi droga jest pusta to: priority = 0
  - Jeśli droga nie jest pusta, a na drodze jest światło czerwone to priorytet jest wyliczany według wzoru:

    `priority` = (`car_count` * `PRIORITY_CAR_COUNT`) + (`light_step_counter` * `PRIORITY_LIGHT_STEP_COUNTER`);

  - Jeśli droga nie jest pusta, a na drodze jest światło zielone to priorytet jest wyliczany według wzoru:

    `priority` = (`car_count` * `PRIORITY_CAR_COUNT`)

    `car_count` - liczba samochodów na drodze
    `light_step_counter` - liczba kroków symulacji od ilu trwa swiatlo czerwone, liczone od momentu pojawienia sie pojazdu na drodze (zmienna nie jest inkrementowana jeśli droga stoi pusta)
    `PRIORITY_CAR_COUNT` - waga współczynnika `car_count`, domyślnie 2
    `PRIORITY_LIGHT_STEP_COUNTER` - waga współczynnika `light_step_counter`, domyślnie 1

3. Wybór drogi z najwyższym priorytetem. Jeśli wystąpi więcej niż jedna droga z takim priorytetem to wybrana zostaje jedna z nich.

4. Rozpoczęcie procesu zmiany świateł z czerwonego do zielonego, przechodząc przez światło żółte na jeden krok symulacji.

5. Gdy ustawi się czerwone światło następuje rozpoczęcie procesu zmiany świateł na wybranej wcześniej drodze na zielone, również przechodząc przez żółte.

6. Zielone światło traw minimum tyle kroków, ile ustawiono w definicji `TRAFFIC_MIN_GREEN_STEPS`, domyślnie 4

7. Następnie po upływie czasu zielonego światła algorytm się powtarza. Jeśli ponownie wybrana zostaje ta sama droga, to światła zostają zielone (nie wykonują się kroki 4. i 5.)
