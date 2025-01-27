//Parametros de los perfiles NACA (MPTT):
//Primer digito / 100 = M
M = 0.09;
//Segundo digito / 10 = P
P = 0.40;
//Tercer y cuarto digitos / 10 = T
T = 0.12;

//Constantes del grosor del ala:
a0 = 0.2969;
a1 = -0.126;
a2 = -0.3516;
a3 = 0.2843;
a4 = -0.1015; //o -0.1036 para un borde de salida cerrado

//Anclajes
ejeP = 0.4;  //Porcentaje para el eje del anclaje
ejeRadio = 5.2 / 2; //Radio del pasador
ranLong = 41;  //Dimensiones de la ranura
ranAnch = 5.65;
ranAlt = 5.7;

//Precisiones
$fn = 500; // Precisión elementos predefinidos
N = 1000;  // Precisión de la curva

//Tamano total del ala:
longitud = 100;
altura = 150;

//Funciones auxliares:

function inverso(xs) =
  [for (i = [0:len(xs)-1]) xs[len(xs)-1-i] ];

function modulo(v) = sqrt(v[0]^2 + v[1]^2);
  
function unitario(v) = v / modulo(v);

//Funciones de la forma del perfil alar:
  
function comba(x) = x<P
  ? M / P^2 * (2*P*x - x^2)
  : M/(1-P)^2 * (1 - 2*P + 2*P*x - x^2);

function anchura(x) = 
  T/0.2 *(a0*x^0.5 + a1*x + a2*x^2 +a3*x^3 + a4*x^4);
      
function combaPendiente(x) = x<P
  ? 2*M/P^2 * (P-x)
  : 2*M/(1-P)^2 * (P-x);

function combaGradiente(x) = unitario([-combaPendiente(x),1]);
  
module NACA() {
  pa = [ for (i = [0:N]) 
    [i/N,comba(i/N)] + combaGradiente(i/N)*anchura(i/N)];
  pab = [ for (i = [0:N])
    [i/N,comba(i/N)] - combaGradiente(i/N)*anchura(i/N)];
  pf = concat(pa, inverso(pab));
  polygon(pf);
};

// Modelo completo

ejeX = ejeP * longitud;
ejeY = longitud*comba(ejeP);

ranX = ejeX - ranLong/2;
ranY = ejeY - ranAnch/2;
ranZ = altura - ranAlt;
ranC = [longitud, ranAnch, ranAlt+1];

difference() {
  linear_extrude(altura) scale([longitud,longitud]) NACA();

  translate([ejeX, ejeY, -1]) cylinder(h=altura+2, r=ejeRadio);
  translate([ranX, ranY, -1]) cube(ranC);
  translate([ranX, ranY, ranZ]) cube(ranC);
}
