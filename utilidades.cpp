#include "utilidades.h"
#include <iostream>
#include <cstring>
#include <cctype>
#include <ctime>
using namespace std;

// Escribe la fecha de hoy en formato AAAA-MM-DD
void fechaActual(char* destino) {
    time_t ahora = time(0);
    tm* hoy = localtime(&ahora);
    strftime(destino, 11, "%Y-%m-%d", hoy);
}

// La placa debe tener 6 o 7 letras o numeros
bool esPlacaValida(const char* placa) {
    int largo = strlen(placa);
    if (largo < 6 || largo > 7) {
        return false;
    }
    for (int i = 0; i < largo; i++) {
        if (!isalnum(placa[i])) {
            return false;
        }
    }
    return true;
}

// Revisa que la cedula sea solo numeros
// El prefijo V-, E- o J- es opcional (asi lo dice el PDF, seccion 7.1)
bool esCedulaValida(const char* cedula) {
    int inicio = 0;  // desde donde empiezan los numeros

    // Si empieza con V, E o J, despues tiene que venir un guion
    char primera = toupper(cedula[0]);
    if (primera == 'V' || primera == 'E' || primera == 'J') {
        if (cedula[1] != '-') {
            return false;
        }
        inicio = 2;  // los numeros empiezan despues del guion
    }

    // Tiene que haber al menos un numero
    if (cedula[inicio] == '\0') {
        return false;
    }

    // Todo lo que queda tiene que ser numero
    for (int i = inicio; cedula[i] != '\0'; i++) {
        if (!isdigit(cedula[i])) {
            return false;
        }
    }
    return true;
}

// La fecha debe ser AAAA-MM-DD, con mes y dia que existan
bool esFechaValida(const char* fecha) {
    if (strlen(fecha) != 10) {
        return false;
    }
    if (fecha[4] != '-' || fecha[7] != '-') {
        return false;
    }
    for (int i = 0; i < 10; i++) {
        if (i == 4 || i == 7) {
            continue;
        }
        if (!isdigit(fecha[i])) {
            return false;
        }
    }
    int mes = (fecha[5] - '0') * 10 + (fecha[6] - '0');
    int dia = (fecha[8] - '0') * 10 + (fecha[9] - '0');
    if (mes < 1 || mes > 12) {
        return false;
    }
    if (dia < 1 || dia > 31) {
        return false;
    }
    return true;
}
// Revisa si "buscado" aparece dentro de "texto", sin importar mayusculas
bool contieneSinMayusculas(const char* texto, const char* buscado) {
    int largoTexto = strlen(texto);
    int largoBuscado = strlen(buscado);
    for (int i = 0; i + largoBuscado <= largoTexto; i++) {
        bool coincide = true;
        for (int j = 0; j < largoBuscado; j++) {
            if (tolower(texto[i + j]) != tolower(buscado[j])) {
                coincide = false;
                break;
            }
        }
        if (coincide) {
            return true;
        }
    }
    return false;
}
// Pide un numero y repite si escriben letras o si esta fuera de rango
int leerEntero(const char* mensaje, int minimo, int maximo) {
    int valor;
    while (true) {
        cout << mensaje;
        if (cin >> valor && valor >= minimo && valor <= maximo) {
            cin.ignore(10000, '\n');
            return valor;
        }
        cout << "Valor invalido. Intente de nuevo." << endl;
        cin.clear();
        cin.ignore(10000, '\n');
    }
}

// Pide un texto. Devuelve false si escriben CANCELAR
bool leerTexto(const char* mensaje, char* destino, int tamano) {
    string linea;
    cout << mensaje;
    getline(cin, linea);
    if (linea == "CANCELAR") {
        return false;
    }
    strncpy(destino, linea.c_str(), tamano - 1);
    destino[tamano - 1] = '\0';
    return true;
}

// Pregunta S/N y devuelve true si responden S
bool confirmar(const char* mensaje) {
    char respuesta;
    while (true) {
        cout << mensaje << " (S/N): ";
        cin >> respuesta;
        cin.ignore(10000, '\n');
        respuesta = toupper(respuesta);
        if (respuesta == 'S') {
            return true;
        }
        if (respuesta == 'N') {
            return false;
        }
        cout << "Responda S o N." << endl;
    }
}