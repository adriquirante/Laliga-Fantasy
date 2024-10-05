#include <iostream>
#include <vector>
#include <fstream>
#include <cassert>
#include <algorithm>
#include <chrono>
#include <iomanip>
using namespace std;

/*Variables globals*/
string outputFile;
chrono::_V2::system_clock::time_point start;

class Player {
public:
    int id;
    string name;
    string position;
    int price;
    string club;
    int points;

    // Mètodes
    Player(int _id, string _name, string _position, int _price, string _club, int _points):
        id(_id), name(_name), position(_position), price(_price), club(_club), points(_points){}
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

    int numPor() {
        return porter.size();
    }

    int numDef() {
        return defenses.size();
    }

    int numMig() {
        return migcampistes.size();
    }

    int numDav() {
        return davanters.size();
    }

    //Aquestes funcions afegeixen un jugador a l'equip
    void afageixPor(const Player& jugador) {
        porter.push_back(jugador);
        preu += jugador.price;
        puntuacio += jugador.points;
    }

    void afageixDef(const Player& jugador) {
        defenses.push_back(jugador);
        preu += jugador.price;
        puntuacio += jugador.points;
    }

    void afageixMig(const Player& jugador) {
        migcampistes.push_back(jugador);
        preu += jugador.price;
        puntuacio += jugador.points;
    }

    void afageixDav(const Player& jugador) {
        davanters.push_back(jugador);
        preu += jugador.price;
        puntuacio += jugador.points;
    }

    //Aquestes funcions retornen si un jugador és pitjor que tots aquells que estan en el equip i juguen a la mateixa posicio
    bool pitjor_por(const Player& jugador) const {
        for (const auto& por : porter) {
            if (por.points >= jugador.points and por.price <= jugador.price) return true;
        }
        return false;
    }    
    bool pitjor_def(const Player& jugador) const {
        for (const auto& defensa : defenses) {
            if (defensa.points > jugador.points and defensa.price < jugador.price) return true;
        }
        return false;
    }
    bool pitjor_mig(const Player& jugador) const {
        for (const auto& mig : migcampistes) {
            if (mig.points > jugador.points and mig.price <  jugador.price) return true;
        }
        return false;
    }
    bool pitjor_dav(const Player& jugador) const {
        for (const auto& dav : davanters) {
            if (dav.points > jugador.points and dav.price < jugador.price) return true;
        }
        return false;
    }
    
    // Afageix/treu punts i preu a l'equip, quan afegim/traiem un jugador a l'equip
    void suma(const Player& jugador) {
        preu += jugador.price;
        puntuacio += jugador.points;
    }
    void resta(const Player& jugador) {
        preu -= jugador.price;
        puntuacio -= jugador.points;
    }
};

class Tactic {
public:
    int nPor;
    int nDef;
    int nMig;
    int nDav;
    int maxPreuTotal;
    int maxPreuIndiv;

    //Mètodes
    Tactic(int _por, int _def, int _mig, int _dav, int _maxPreuTotal, int _maxPreuIndiv):
        nPor(_por), nDef(_def), nMig(_mig), nDav(_dav), maxPreuTotal(_maxPreuTotal), maxPreuIndiv(_maxPreuIndiv){}

    bool valid(int preu_jugador) const {
        return preu_jugador <= maxPreuIndiv;
    }

    bool ens_hem_passat(const Equip& equip) const {
        return equip.preu > maxPreuTotal;
    }
};

//Llegim la base de dades i ho guardem en una matriu amb 4 files: una per porters, defenses, migcampistes i davanters
vector<vector<Player>> read_database(const string& database, const Tactic& restriccions) {
    vector<Player> porters;
    vector<Player> defenses;
    vector<Player> migcampistes;
    vector<Player> davanters;
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

        if (price > 0 and p == 0);
        else if (restriccions.valid(price)) {
            if (position == "por") porters.push_back(Player(nextId++,name,position,price,club,p));
            else if (position == "def") defenses.push_back(Player(nextId++,name,position,price,club,p));
            else if (position == "mig") migcampistes.push_back(Player(nextId++,name,position,price,club,p));
            else if (position == "dav") davanters.push_back(Player(nextId++,name,position,price,club,p));
        }

    }
    in.close();
    jugadors.push_back(porters);
    jugadors.push_back(defenses);
    jugadors.push_back(migcampistes);
    jugadors.push_back(davanters);
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

void escriu(const Equip& equip) {
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

//Funció que genera les solucions
void genera(const vector<vector<Player>>& database, const Tactic& restriccions, Equip& millor_equip, Equip& equip_actual, 
int idx, uint idx_por, uint idx_def, uint idx_mig, uint idx_dav) {

    if (restriccions.ens_hem_passat(equip_actual)) return;
    else if (uint(1 - equip_actual.numPor()) > database[0].size() - idx_por+1) return;
    else if (uint(restriccions.nDef - equip_actual.numDef()) > database[1].size() - idx_def+1) return;
    else if (uint(restriccions.nMig - equip_actual.numMig()) > database[2].size() - idx_mig+1) return;
    else if (uint(restriccions.nDav - equip_actual.numDav()) > database[3].size() - idx_dav+1) return;
    else if (idx == 11) { 
        if (equip_actual.puntuacio > millor_equip.puntuacio) {
            millor_equip = equip_actual;
            escriu(equip_actual);
        }
    }
    //Busquem porter
    else if ((idx < 1) and (idx_por < database[0].size())) {
        if (not millor_equip.pitjor_por(database[0][idx_por])) {
            equip_actual.afageixPor(database[0][idx_por]);
            genera(database, restriccions, millor_equip, equip_actual, idx+1, idx_por+1, idx_def, idx_mig, idx_dav);
            equip_actual.porter.pop_back();
            equip_actual.resta(database[0][idx_por]);
        }
        
        genera(database, restriccions, millor_equip, equip_actual, idx, idx_por+1, idx_def, idx_mig, idx_dav);
    }

    //Busquem defenses
    else if ((idx < 1+restriccions.nDef) and (idx_def < database[1].size())) {
        equip_actual.afageixDef(database[1][idx_def]);
        genera(database, restriccions, millor_equip, equip_actual, idx+1, idx_por, idx_def+1, idx_mig, idx_dav);
        equip_actual.defenses.pop_back();
        equip_actual.resta(database[1][idx_def]);
        
        genera(database, restriccions, millor_equip, equip_actual, idx, idx_por, idx_def+1, idx_mig, idx_dav);
    }

    //Busquem migcampistes
    else if ((idx < 1+restriccions.nDef+restriccions.nMig) and (idx_mig < database[2].size())) {
        equip_actual.afageixMig(database[2][idx_mig]);
        genera(database, restriccions, millor_equip, equip_actual, idx+1, idx_por, idx_def, idx_mig+1, idx_dav);
        equip_actual.migcampistes.pop_back();
        equip_actual.resta(database[2][idx_mig]);
        
        genera(database, restriccions, millor_equip, equip_actual, idx, idx_por, idx_def, idx_mig+1, idx_dav);
    }

    //Busquem davanters
    else if ((idx < 11) and (idx_dav < database[3].size())) {
        equip_actual.afageixDav(database[3][idx_dav]);
        genera(database, restriccions, millor_equip, equip_actual, idx+1, idx_por, idx_def, idx_mig, idx_dav+1);
        equip_actual.davanters.pop_back();
        equip_actual.resta(database[3][idx_dav]);

        genera(database, restriccions, millor_equip, equip_actual, idx, idx_por, idx_def, idx_mig, idx_dav+1);
    }
}

//Funció que genera les solucions
void genera(const vector<vector<Player>>& database, const Tactic& restriccions) {
    Equip millor_equip;
    Equip equip_actual;

    genera(database, restriccions, millor_equip, equip_actual, 0, 0, 0, 0, 0);
}

int main(int argc, char** argv) {
    start = std::chrono::high_resolution_clock::now();
    Tactic restriccions = read_query(argv[2]);
    vector<vector<Player>> database = read_database(argv[1], restriccions);
    outputFile = argv[3];
    
    genera(database, restriccions);
}