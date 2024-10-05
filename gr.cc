#include <iostream>
#include <vector>
#include <fstream>
#include <cassert>
#include <algorithm>
#include <chrono>
#include <iomanip>
using namespace std;

//Variable global
chrono::_V2::system_clock::time_point start;

class Player {
public:
    int id;
    string name;
    string position;
    int price;
    string club;
    int points;
    double ponderacio; //points*points*(points/price)

    // Mètodes
    Player(int _id, string _name, string _position, int _price, string _club, int _points, double _ponderacio):
        id(_id), name(_name), position(_position), price(_price), club(_club), points(_points), ponderacio(_ponderacio){}
};


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

    void afageix(const Player& jugador) {
        if (jugador.position == "por") porter.push_back(jugador);
        else if (jugador.position == "def") defenses.push_back(jugador);
        else if (jugador.position == "mig") migcampistes.push_back(jugador);
        else if (jugador.position == "dav") davanters.push_back(jugador);

        preu += jugador.price;
        puntuacio += jugador.points;
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

    bool valid(int preu_jugador) const {
        return preu_jugador <= maxPreuIndiv;
    }

    bool ensHemPassat(const Equip& equip, const Player& jugador) const {
        return equip.preu + jugador.price > maxPreuTotal;
    }
};

//Funció per ordenar una llista de jugadors
bool before(const Player& jugador1, const Player& jugador2) {
    if (jugador1.ponderacio > jugador2.ponderacio) return true;
    return false;
}


//Llegim la base de dades i ho guardem en un vector de Players
vector<Player> read_database(const string& database, const Tactic& restriccions) {
    vector<Player> jugadors;

    ifstream in(database);
    int nextId = 0;
    while(not in.eof()) {
        string name, club, position;
        int points;
        getline(in,name,';');    if (name == "") break;
        getline(in,position,';');
        int price; in >> price;
        char aux; in >> aux;
        getline(in,club,';');
        in >> points;
        string aux2;
        getline(in,aux2);

        //Càlcul de la ponderació
        double ponderacio;
        if (price == 0) ponderacio = points;
        else if (points == 0 and price > 0) ponderacio = -1;
        else ponderacio = points*points*(double(points)/double(price));

        if (restriccions.valid(price)) jugadors.push_back(Player(nextId++, name, position, price, club, points, ponderacio));
    }
    in.close();

    return jugadors;
}


//Llegim les restriccions
Tactic read_query(const string& query) {
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
        if (equip.porter.size() < restriccions.nPor) return true;
    }
    else if (jugador.position == "def") {
        if (equip.defenses.size() < restriccions.nDef) return true;
    }
    else if (jugador.position == "mig") {
        if (equip.migcampistes.size() < restriccions.nMig) return true;
    }
    else if (jugador.position == "dav") {
        if (equip.davanters.size() < restriccions.nDav) return true;
    }

    return false;
}

void escriu(const Equip& equip, const string& outputFile) {

    ofstream output(outputFile);
    auto end = chrono::high_resolution_clock::now();
    auto duracio = chrono::duration_cast<std::chrono::milliseconds>(end - start);
    output << fixed << setprecision(1) << duracio.count() << endl;

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

//Genera una solució greedy
void millorEquip(vector<Player>& database, const Tactic& restriccions, const string& outputFile) {

    int idxEquip = 0, idxJugador = 0;
    Player jugador(-1, "", "", -1, "", -1, -1);
    Equip equip;

    while (idxEquip < 11) {
        jugador = database[idxJugador];
        if (apte(restriccions, equip, jugador)) {
            equip.afageix(jugador);
            ++idxEquip;
        }
        ++idxJugador;
    }

    escriu(equip, outputFile);
}


int main(int argc, char** argv) {
    Tactic restriccions = read_query(argv[2]);
    vector<Player> database = read_database(argv[1], restriccions);
    start = std::chrono::high_resolution_clock::now();

    sort(database.begin(), database.end(), before);
    millorEquip(database, restriccions, argv[3]);
}