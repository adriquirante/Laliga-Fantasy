#include <iostream>
#include <vector>
#include <fstream>
//#include <cassert>
#include <algorithm>
#include <chrono>
#include <cmath>
//#include <cstdlib>   
//#include <ctime>
//#include <iomanip>
#include <random>

using namespace std;
using namespace chrono;


class Player {
public:
    int id;
    string name;
    string position;
    int price;
    string club;
    int points;
    double ponderacio;

    // Mètodes
    Player(int _id, string _name, string _position, int _price, string _club, int _points, double _ponderacio):
        id(_id), name(_name), position(_position), price(_price), club(_club), points(_points), ponderacio(_ponderacio){}
};

//Variables globals
vector<Player> jugadorsOrdenats; //vector de jugadors ordenats descendentment segons el seu valor de ponderació
string outputFile;
time_point<high_resolution_clock> start;



class Equip {
public:
    vector<Player> porter;
    vector<Player> defenses;
    vector<Player> migcampistes;
    vector<Player> davanters;
    int preu;
    int puntuacio;

    //Mètodes
    Equip(): preu(0), puntuacio(0){}

    bool millor(const Equip& equip) {
       return puntuacio > equip.puntuacio;
    }

    void afageix(const Player& jugador) {
        if (jugador.position == "por") porter.push_back(jugador);
        else if (jugador.position == "def") defenses.push_back(jugador);
        else if (jugador.position == "mig") migcampistes.push_back(jugador);
        else if (jugador.position == "dav") davanters.push_back(jugador);

        preu += jugador.price;
        puntuacio += jugador.points;
    }

    void suma(const Player& jugador) {
        preu += jugador.price;
        puntuacio += jugador.points;
    }

    void resta(const Player& jugador) {
        preu -= jugador.price;
        puntuacio -= jugador.points;
    }

    //Mira si un jugador que volem afegir a l'equip ja hi és
    bool porRepetit(const Player& jugador) {
        return jugador.id == porter[0].id;
    }

    bool defRepetit(const Player& jugador) {
        for (uint i = 0; i < defenses.size(); ++i) {
            if (jugador.id == defenses[i].id) return true;
        }
        return false;
    }

    bool migRepetit(const Player& jugador) {
        for (uint i = 0; i < migcampistes.size(); ++i) {
            if (jugador.id == migcampistes[i].id) return true;
        }
        return false;
    }

    bool davRepetit(const Player& jugador) {
        for (uint i = 0; i < davanters.size(); ++i) {
            if (jugador.id == davanters[i].id) return true;
        }
        return false;
    }
};


class Tactic {
public:
    uint nPor;
    uint nDef;
    uint nMig;
    uint nDav;
    int maxPreuTotal;
    int maxPreuIndiv;

    //Mètodes
    Tactic(int _por, int _def, int _mig, int _dav, int _maxPreuTotal, int _maxPreuIndiv):
        nPor(_por), nDef(_def), nMig(_mig), nDav(_dav), maxPreuTotal(_maxPreuTotal), maxPreuIndiv(_maxPreuIndiv){}

    bool jugadorValid(int preu_jugador) const {
        return preu_jugador <= maxPreuIndiv;
    }

    bool ensHemPassat(const Equip& equip, const Player& jugador) const {
        return equip.preu + jugador.price > maxPreuTotal;
    }
};


/*Funcions*/

// Considerem un jugador  "inutil" si fitxar-lo costa diners i té una puntuació igual a 0
bool inutil(int preu, int punts) {
        return preu > 0 and punts == 0;
    }

//Funció per ordenar una llista de jugadors
bool before(const Player& jugador1, const Player& jugador2) {
    if (jugador1.ponderacio > jugador2.ponderacio) return true;
    return false;
}

/*Llegim la base de dades i ho guardem en una matriu amb 4 files: una per porters, defenses, migcampistes i davanters
A més, modifica la variable global "jugadorsOrdenats" que necessitarem per fer algunes operacions*/
vector<vector<Player>> readDatabase(const string& database, const Tactic& restriccions) {
    vector<Player> porters, defenses, migcampistes, davanters;
    vector<vector<Player>> jugadors;

    ifstream in(database);
    int nextId = 0;
    while(not in.eof()) {
        string name, club, position;
        int p;
        getline(in,name,';');    if (name == "") break;
        getline(in,position,';');
        int price; in >> price;
        char aux; in >> aux;
        getline(in,club,';');
        in >> p;
        string aux2;
        getline(in,aux2);

        //Càlcul de la ponderació
        double ponderacio;
        if (price == 0) ponderacio = p;
        else if (p == 0 and price > 0) ponderacio = -1;
        else ponderacio = p*p*(double(p)/double(price));

        if (restriccions.jugadorValid(price) and not inutil(price, p)) {
            nextId += 1;
            if (position == "por") porters.push_back(Player(nextId,name,position,price,club,p,ponderacio));
            else if (position == "def") defenses.push_back(Player(nextId,name,position,price,club,p,ponderacio));
            else if (position == "mig") migcampistes.push_back(Player(nextId,name,position,price,club,p,ponderacio));
            else if (position == "dav") davanters.push_back(Player(nextId,name,position,price,club,p,ponderacio));
            jugadorsOrdenats.push_back(Player(nextId,name,position,price,club,p,ponderacio));
        }

    }
    in.close();
    
    sort(jugadorsOrdenats.begin(), jugadorsOrdenats.end(), before);
    
    jugadors.push_back(porters);
    jugadors.push_back(defenses);
    jugadors.push_back(migcampistes);
    jugadors.push_back(davanters);
    
    return jugadors;
}

//Llegim les restriccions
Tactic readQuery(const string& query) {
    ifstream in(query);
    int nDef, nMig, nDav;
    int maxTotalPrice, maxIndivPrice;
    in >> nDef >> nMig >> nDav >> maxTotalPrice >> maxIndivPrice;
    Tactic restriccions(1, nDef, nMig, nDav, maxTotalPrice, maxIndivPrice);
    in.close();

    return restriccions;
}

//Donat un jugador, diu si aquest pot ser afegit a l'equip o no
bool apte(const Tactic& restriccions, const Equip& equip, const Player& jugador) {

    if (restriccions.ensHemPassat(equip,jugador)) return false;

    else if (jugador.position == "por") {
        if (equip.porter.size() < restriccions.nPor-1) return true;
    }
    else if (jugador.position == "def") {
        if (equip.defenses.size() < restriccions.nDef-1) return true;
    }
    else if (jugador.position == "mig") {
        if (equip.migcampistes.size() < restriccions.nMig-1) return true;
    }
    else if (jugador.position == "dav") {
        if (equip.davanters.size() < restriccions.nDav-1) return true;
    }

    return false;
}

void escriu2(const Equip& equip) {   

    cout << "POR: " << equip.porter[0].name << endl;

    bool first = true;
    cout << "DEF: ";
    for (uint i = 0; i < equip.defenses.size(); ++i) {
        if (first) {
            cout<< equip.defenses[i].name;
            first = false;
        }
        else cout << ";" << equip.defenses[i].name;
    }
    cout << endl;

    first = true;
    cout << "MIG: ";
    for (uint i = 0; i < equip.migcampistes.size(); ++i) {
        if (first) {
            cout << equip.migcampistes[i].name;
            first = false;
        }
        else cout << ";"  << equip.migcampistes[i].name;
    }
    cout << endl;

    first = true;
    cout << "DAV: ";
    for (uint i = 0; i < equip.davanters.size(); ++i) {
        if (first) {
            cout << equip.davanters[i].name;
            first = false;
        }
        else cout << ";" << equip.davanters[i].name;
    }
    cout << endl;

    cout << "Punts: " << equip.puntuacio << endl;
    cout << "Preu: " << equip.preu << endl;
}

void escriu(const Equip& equip) {
    ofstream output(outputFile);
    time_point<high_resolution_clock> end = high_resolution_clock::now();
    duration<double> duracio = end - start;
    double duracio_a = round(duracio.count()*10.0)/10.0;

    output << duracio_a << endl;
    

    output << "POR: " << equip.porter[0].name << endl;

    bool first = true;
    output << "DEF: ";
    for (uint i = 0; i < equip.defenses.size(); ++i) {
        if (first) {
            output << equip.defenses[i].name;
            first = false;
        }
        else output << ";" << equip.defenses[i].name;
    }
    output << endl;

    first = true;
    output << "MIG: ";
    for (uint i = 0; i < equip.migcampistes.size(); ++i) {
        if (first) {
            output << equip.migcampistes[i].name;
            first = false;
        }
        else output << ";" << equip.migcampistes[i].name;
    }
    output << endl;

    first = true;
    output << "DAV: ";
    for (uint i = 0; i < equip.davanters.size(); ++i) {
        if (first) {
            output << equip.davanters[i].name;
            first = false;
        }
        else output << ";" << equip.davanters[i].name;
    }
    output << endl;

    output << "Punts: " << equip.puntuacio << endl;
    output << "Preu: " << equip.preu << endl;
}

//Genera número random entre 0 i "limit"
int numRandom(int limit) {
    random_device rd;
    mt19937 generator(rd());
    uniform_int_distribution<int> distribution(0, limit-1);
    return distribution(generator);
}

//Genera una solució greedy random que correspondrà a la solució inicial del nostre algorisme "Grasp"
Equip equipGreedyRandom(const vector<vector<Player>>& database, const Tactic& restriccions) {

    //Busquem la nostra solució greedy, exceptuant un jugador de cada posició
    int idxEquip = 0, idxJugador = 0;
    Player jugador(-1, "", "", -1, "", -1, -1);
    Equip equip;

    while (idxEquip < 7) {
        jugador = jugadorsOrdenats[idxJugador];
        if (apte(restriccions, equip, jugador)) {
            equip.afageix(jugador);
            ++idxEquip;
        }
        ++idxJugador;
    }

    //Afegim un porter, un defensa, un migcampista i un davanter aleatòriament
    jugador = database[0][numRandom(database[0].size())];
    while (restriccions.ensHemPassat(equip, jugador)) jugador = database[0][numRandom(database[0].size())];
    equip.afageix(jugador);
    
    jugador = database[1][numRandom(database[1].size())];
    while (restriccions.ensHemPassat(equip, jugador) or equip.defRepetit(jugador)) jugador = database[1][numRandom(database[1].size())];
    equip.afageix(jugador);

    jugador = database[2][numRandom(database[2].size())];
    while (restriccions.ensHemPassat(equip, jugador) or equip.migRepetit(jugador)) jugador = database[2][numRandom(database[2].size())];
    equip.afageix(jugador);

    jugador = database[3][numRandom(database[3].size())];
    while (restriccions.ensHemPassat(equip, jugador) or equip.davRepetit(jugador)) jugador = database[3][numRandom(database[3].size())];
    equip.afageix(jugador);

    return equip;
}

Player buscaJugador(const vector<vector<Player>>& database, const Tactic& restriccions, Equip& equipNou, const string& pos) {

    Player millorJugador(-1,"","",-1,"",-1,-1);

    if (pos == "por") {
        for (uint i = 0; i < database[0].size(); ++i) {
            if (not restriccions.ensHemPassat(equipNou, database[0][i]) and database[0][i].points > millorJugador.points and 
            not equipNou.porRepetit(database[0][i])) millorJugador =  database[0][i];
        }
    }

    else if (pos == "def") {
        for (uint i = 0; i < database[1].size(); ++i) {
            if (not restriccions.ensHemPassat(equipNou, database[1][i]) and database[1][i].points > millorJugador.points and 
            not equipNou.defRepetit(database[1][i])) millorJugador = database[1][i];
        }
    }

    else if (pos == "mig") {
        for (uint i = 0; i < database[2].size(); ++i) {
            if (not restriccions.ensHemPassat(equipNou, database[2][i]) and database[2][i].points > millorJugador.points and
            not equipNou.migRepetit(database[2][i])) millorJugador = database[2][i];
        }
    }

    else if (pos == "dav") {
        for (uint i = 0; i < database[3].size(); ++i) {
            if (not restriccions.ensHemPassat(equipNou, database[3][i]) and database[3][i].points > millorJugador.points and
            not equipNou.davRepetit(database[3][i])) millorJugador = database[3][i];
            }
        }

    return millorJugador;
}

Equip generaEquip(const vector<vector<Player>>& database, const Tactic& restriccions, const Equip& millorVeinat, uint idx) {

    //Creem un equip traient el jugador "k" de l'equipInicial i posant un "millorJugador"
    Equip equipNou = millorVeinat;
    Player millorJugador(-1,"","",-1,"",-1,-1);
    
    if (idx == 0) {
        equipNou.resta(millorVeinat.porter[0]);
        millorJugador = buscaJugador(database, restriccions, equipNou, "por");

        //Si el jugador trobat puntua més que el que teníem, el substitueix, sinó ho deixa tot igual
        if (millorJugador.points < millorVeinat.porter[0].points) equipNou.suma(millorVeinat.porter[0]);
        else{
            equipNou.porter[0] = millorJugador;
            equipNou.suma(millorJugador);
        }
    }
    
    else if (idx <= restriccions.nDef) {
        equipNou.resta(millorVeinat.defenses[idx-1]);
        millorJugador = buscaJugador(database, restriccions, equipNou, "def");

        //Si el jugador trobat puntua més que el que teníem, el substitueix, sinó ho deixa tot igual
        if (millorJugador.points < equipNou.defenses[idx-1].points) equipNou.suma(equipNou.defenses[idx-1]);
        else{
            equipNou.defenses[idx-1] = millorJugador;
            equipNou.suma(millorJugador);
        }
    }

    else if (idx <= restriccions.nDef + restriccions.nMig) {
        equipNou.resta(millorVeinat.migcampistes[idx - 1 - restriccions.nDef]);
        millorJugador = buscaJugador(database, restriccions, equipNou, "mig");

        //Si el jugador trobat puntua més que el que teníem, el substitueix, sinó ho deixa tot igual
        if (millorJugador.points < equipNou.migcampistes[idx - 1 - restriccions.nDef].points) equipNou.suma(equipNou.migcampistes[idx - 1 - restriccions.nDef]);
        else{
            equipNou.migcampistes[idx - 1 - restriccions.nDef] = millorJugador;
            equipNou.suma(millorJugador);
        }
    }

    else {
        equipNou.resta(millorVeinat.davanters[idx - 1 - restriccions.nDef - restriccions.nMig]);
        millorJugador = buscaJugador(database, restriccions, equipNou, "dav");

        //Si el jugador trobat puntua més que el que teníem, el substitueix, sinó ho deixa tot igual
        if (millorJugador.points < equipNou.davanters[idx - 1 - restriccions.nDef - restriccions.nMig].points) equipNou.suma(equipNou.davanters[idx - 1 - restriccions.nDef - restriccions.nMig]);
        else{
            equipNou.davanters[idx - 1 - restriccions.nDef - restriccions.nMig] = millorJugador;
            equipNou.suma(millorJugador);
        }
    }
    
    return equipNou;
}

void neighbourhood_search(const vector<vector<Player>>& database, const Tactic& restriccions, Equip& equipInicial, Equip& millorEquip) {

    int k = 0;
    int idx;
    vector<bool> usats(11, false);
    Equip equipNou;
    Equip millorVeinat = equipInicial;
    /*
    while (k < 11) {
        //Busquem aleatòriament la posició candidata que volem modificar
        idx = numRandom(11);
        while (usats[idx]) idx = numRandom(11);
        usats[idx] = true;

        //Busquem un nou equip modificant el millorVeinat en la posició idx
        equipNou = generaEquip(database, restriccions, millorVeinat, uint(idx));

        //Si trobem un millorVeinat tornem a començar el bucle, sinó seguim
        if (equipNou.millor(millorVeinat)) {
            millorVeinat = equipNou;
            k = 0;
            for (int i = 0; i < 11; ++i) {
                usats[i] = false;
            }

            if (millorVeinat.millor(millorEquip)) {
                millorEquip = millorVeinat;
                escriu2(millorEquip);
            }
        }
        else k += 1;
    }*/

    for (int k = 0; k < 11; ++k) {
        equipNou = generaEquip(database, restriccions, millorVeinat, uint(k));
        if (equipNou.millor(millorVeinat)) {
            millorVeinat = equipNou;

            if (millorVeinat.millor(millorEquip)) {
                millorEquip = millorVeinat;
                escriu2(millorEquip);
            }
        }
    }
}

void grasp(const vector<vector<Player>>& database, const Tactic& restriccions) {

    //1a fase
    Equip equipInicial = equipGreedyRandom(database, restriccions);
    Equip millorEquip = equipInicial;
    escriu2(millorEquip);

    //2a fase
    while (true) {
        neighbourhood_search(database, restriccions, equipInicial, millorEquip);
        equipInicial = equipGreedyRandom(database, restriccions);
    }
}



int main(int argc, char** argv){
    start = high_resolution_clock::now();

    //Llegim les restriccions, la database i l'outputFile
    Tactic restriccions = readQuery(argv[2]);
    vector<vector<Player>> database = readDatabase(argv[1], restriccions);
    outputFile = argv[3];

    //Generem solucions
    grasp(database, restriccions);
    
}