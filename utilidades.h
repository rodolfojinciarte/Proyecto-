#pragma once
#include <string>

void fechaActual(char* destino);
bool esFechaValida(const char* fecha);
bool esPlacaValida(const char* placa);
bool esCedulaValida(const char* cedula);
bool contieneSinMayusculas(const char* texto, const char* buscado);
int leerEntero(const char* mensaje, int minimo, int maximo);
bool leerTexto(const char* mensaje, char* destino, int tamano);
bool confirmar(const char* mensaje);