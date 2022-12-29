




/*
   WEllington Ferreira de oliveira
   Data 02/03/2021 conforme as alteraçoes
   Utilizando NTP como serviço https://www.ntppool.org/ de data hora
    Selecionar metodo de Setar o fuso horario veja os exemplos:
    GMT +1 = 3600
    GMT +8 = 28800
    GMT -1 = -3600
    GMT 0 = 0
    timeClient.setTimeOffset(0);
    timeClient.setTimeOffset(-10500);

   Utilizando Telegran como Serviço de Recebimento de mensagens https://t.me/botfather

   Para enviar e-mail usando o Gmail, use a porta 465 (SSL) e o servidor SMTP smtp.gmail.com
   VOCÊ DEVE ATIVAR a opção de aplicativo menos segura https://myaccount.google.com/lesssecureapps?pli=1

   Para utilizar SD CARD seguintes intrucoes
   Crie um arquivo TXT chamado de ( alarme.txt )
   Crie um arquivo TXT chamado de ( reseta.txt )

*/

#include "ESP32_MailClient.h"     // Biblioteca Email-envio de mensagem //
#include <UniversalTelegramBot.h> // Biblioteca botfather telegran //
#include <WiFiClientSecure.h>     // Biblioteca Wifi de Seguranca //
#include <WiFi.h>                 // Wifi comunicacao wifi com esp32 //
#include "DHT.h"                  // Temperatura Ambiente biblioteca DHT //


#include "FS.h"                   // biblioteca SDCARD //
#include "SD.h"                   // biblioteca SDCARD //
#include "SPI.h"                  // biblioteca SDCARD //

#include <WiFiUdp.h>              // Server UDP para SDCARD //
#include <NTPClient.h>            // biblioteca Server NTP para ser usado no SDCARD //


#define DHTPIN 14
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);


// Acesso ao Roteador senha e login //
const char* ssid = "00000000000000";
const char* password = "0000000";

#define emailSenderAccount   "wellington.ferreira1259@gmail.com"
#define emailSenderPassword  "0000000"
#define emailRecipient1      "wellington.ferreira12@hotmail.com" // Varia as quantidades de emails enviado //
#define emailRecipient2      "claudia.bispo@outlook.com"         // Varia as quantidades de emails enviado secundario //
#define smtpServer           "smtp.gmail.com"
#define smtpServerPort        465
#define emailSubject         "Alerta Seu Alarme Esta Detectando Alguem..."


#define BOTtoken "1413011516:AAG-J9pIA-MJfcaAitKOFnyQ-FALh6tgWxo" // numero token Telegran criado no bootfahter //


WiFiClientSecure client;   // Cliente de Segurança do Servidor //


UniversalTelegramBot bot(BOTtoken, client); // Pegando o valor TOKEN Universal //


// Configurações do Servidor NTP
// Fuso horário em segundos (-03h = -10800 seg)

char semana[7][12] = {"Domingo", "Segunda", "Terça", "Quarta", "Quinta", "Sexta", "Sabado"};

WiFiUDP ntpUDP; // Declaração do Protocolo UDP
//NTPClient timeClient(ntpUDP, "3.br.pool.ntp.org", -3 * 3600, 60000); // https://www.ntppool.org/
NTPClient timeClient(ntpUDP, "3.br.pool.ntp.org"); // https://www.ntppool.org/


const int Buzz =   33; // Buzzer de inf do estado do sistema //

const int Red =    25; // led Vermelho //
const int Green =  26; //led verde //
const int Blue =   27; //led Azul //

const int Sirene = 16; // Sirene do alarme Ativo //

const int botao =  32; // botao de ativacao dos sensores //

const int PIR =    21; // snsor Pir de presenca infra vermelho 1 //
const int PIR2 =   22; // Pir2 Desativado por falta de sensor //

const int rele4 =   4;   // entrada de Rele lampada //
const int rele15 = 15; // entrada de Rele lampada //




bool ativacao; // botao que liga as funcoes dos sensores //
int vezes = 0; // liga o aparelho buzz/Led Blue funcao While Millis toca 5X//


char id[3][11] = { "1474733370", "1517068805", "0123456789" }; //id = 1474733370 ; //id = 1517068805 ; // RGs do Tefefones Telegran Recebidos //

unsigned long gatilho = 0;
unsigned long horaatual = millis();  // Hora Atual // web Server
unsigned long tempoanterior = 0;     // Tempo Anterior //  web Server
const long tempocorrente = 2000;     // Defina o tempo limite em milissegundos (exemplo: 2000ms = 2s)  web Server


SMTPData smtpData;
void sendCallback(SendStatus info);


WiFiServer server(80);

String cabecalho;

// Variavel Auxiliar Status, do botao do servidor web//
String rele4estado = "off";
String rele15estado = "off";
String rele4_15estado = "off";
String alarme_estado = "off";


void setup() { // Inicia Setup //

  Serial.begin(115200);
  Serial.print("Conectando... ");
  Serial.println();
  Serial.println(F("DHT11 Para Execucao do Modulo..."));
  Serial.println();

  Serial.println(ssid);
  WiFi.begin(ssid, password);
  // WiFi.config(ip, gateway, subnet); // Alterado conforme para ip Fixo Sempre que entrar //


  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }


  // Inicia o servidor Imprimi ip Local
  Serial.println("");
  Serial.println("Wifi Conectado...");
  Serial.println("IP Endereco... ");
  Serial.println(WiFi.localIP());


  if (!SD.begin()) {
    Serial.println("Falha ao ler cartao SD...!!!");
  }

  //  listDir(SD, "/", 0);
  //  createDir(SD, "/mydir");
  //  listDir(SD, "/", 0);
  //  removeDir(SD, "/mydir");
  //  listDir(SD, "/", 2);

  timeClient.begin();
  timeClient.setTimeOffset(-10500); // NTP configuracao do Fuso Horario //
  server.begin();                   // começa WEBSERVER //
  dht.begin();                      // começa DHT11 temperatura //


  resetasistemaesp(); // grava dados no SD apos o reset //

  pinMode(botao, INPUT_PULLUP);        // Ativa os resistores internos de PULL-UP
  pinMode(PIR,  INPUT_PULLUP);         // Ativa os Sensores internos de PULL-UP

  pinMode(Red,    OUTPUT);              //Define como Saida //
  pinMode(Green,  OUTPUT);             //Define como Saida //
  pinMode(Blue,   OUTPUT);              //Define como Saida //
  pinMode(Buzz,   OUTPUT);              //Define como Saida //
  pinMode(rele4,  OUTPUT);              //Define como Saida //
  pinMode(rele15, OUTPUT);              //Define como Saida //
  pinMode(Sirene, OUTPUT);

  digitalWrite(Red,    LOW); //Define como Saida //
  digitalWrite(Green,  LOW); //Define como Saida //
  digitalWrite(Blue,   LOW); //Define como Saida //
  digitalWrite(Buzz,   LOW); //Define como Saida //
  digitalWrite(rele4,  LOW); //Define como Saida //
  digitalWrite(rele15, LOW); //Define como Saida //
  digitalWrite(Sirene, LOW); //Define como Saida //

  while (vezes < 6) {                   // inicia funcao 5 vezes somente para Reiniciar o sistema  //
    if (tempoanterior == 0) {
      digitalWrite(Buzz, HIGH);
      digitalWrite(Blue, HIGH);         // Liga o Led e Buzz //
      tempoanterior = 1;                // recebe valor 1 //
      gatilho = millis();               // termina tempo //
    } if (millis() - gatilho >= 50 && tempoanterior == 1) {
      digitalWrite(Buzz, LOW);
      digitalWrite(Blue, LOW);
      tempoanterior = 2;               // passa valor para 2 //
    } if (millis() - gatilho >= 150 && tempoanterior == 2) {
      tempoanterior = 0;
      vezes = vezes + 1;
    }
  }
  smtpData.setLogin(smtpServer, smtpServerPort, emailSenderAccount, emailSenderPassword);
  smtpData.setSender("Alarme Acionado...", emailSenderAccount);
  smtpData.setPriority("High");
  smtpData.setSubject(emailSubject);
  smtpData.setMessage("<div style=\"color:#2f4468;\"><h1>Seu Alarme Desparou...</h1><p>- Atenção com a segurança da sua casa...</p></div>", true);
  smtpData.addRecipient(emailRecipient1); // Add Email-1 para envio do cliente //
  smtpData.addRecipient(emailRecipient2); // Add Email-2 para envio do cliente //
  smtpData.setSendCallback(sendCallback);


} // end Setup //



void loop() { // Inicia o Loop //
  timeClient.update();


  // Atualizacao infinita no loop //
  unsigned long epochTime = timeClient.getEpochTime();
  String formattedTime = timeClient.getFormattedTime();
  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();
  int currentSecond = timeClient.getSeconds();
  //Metodo de Estrututa para time //
  struct tm *ptm = gmtime ((time_t *)&epochTime);
  int monthDay = ptm->tm_mday;
  int currentMonth = ptm->tm_mon + 1;
  int currentYear = ptm->tm_year + 1900;

  // Print SD String de Informacoes Gravacoes //
  String horario = timeClient.getFormattedTime();
  String dia = (semana[timeClient.getDay()]);
  String datacompleta = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay);


  if (digitalRead(botao) == LOW) { // o botão for pressionado
    ativacao = ! ativacao;        // Ativa / desativa alarme
    delay(100);                   // Evita acionamentos multiplos Debounce //
    Serial.println("Sensores On...");

  } if (ativacao == true) {       // SE... O alarme estiver ativado...
    Sensor_PIR();
    Ledgreen();
  }
  
  else {
    Serial.println("Sensores Off...");
    LedVermelho();
  }




  WiFiClient client = server.available();

  if (client) {
    horaatual = millis();
    tempoanterior = horaatual;
    Serial.println("Novo Cliente.");
    String linhaatual = "";
    while (client.connected() && horaatual - tempoanterior <= tempocorrente) {
      horaatual = millis();
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        cabecalho += c;
        if (c == '\n') {

          if (linhaatual.length() == 0) {

            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();


            // liga e desliga os GPIOs

            // Liga rele 4 //

            if (cabecalho.indexOf("GET /4/on") >= 0) { // Liga Rele 5 //
              Serial.println("Lampada 1 ON");
              rele4estado = "on";
              digitalWrite(rele4, HIGH);

              // desliga rele 4 //

            } else if (cabecalho.indexOf("GET /4/off") >= 0) { // Desliga Rele 5 //
              Serial.println("Lampada 1 OFF");
              rele4estado = "off";
              digitalWrite(rele4, LOW);

              // Liga rele 15 //

            } else if (cabecalho.indexOf("GET /15/on") >= 0) { // Liga Rele 15 //
              Serial.println("Lampada 15 ON");
              rele15estado = "on";
              digitalWrite(rele15, HIGH);

              // desliga rele 15 //

            } else if (cabecalho.indexOf("GET /15/off") >= 0) { // Desliga Rele 15 //
              Serial.println("Lampada 15 OFF");
              rele15estado = "off";
              digitalWrite(rele15, LOW);

              // Liga rele 4/15 //

            } else if (cabecalho.indexOf("GET /12/on") >= 0) { // Desliga Reles 5-15 //
              Serial.println("Lampada 1/2 ON");
              rele4_15estado = "on";
              digitalWrite(rele4, HIGH);
              digitalWrite(rele15, HIGH);

              // desliga rele 4/15 //

            } else if (cabecalho.indexOf("GET /12/off") >= 0) { // Desliga Rele 5-15 //
              Serial.println("Lampada 1/2 OFF");
              rele4_15estado = "off";
              digitalWrite(rele4, LOW);
              digitalWrite(rele15, LOW);


              // Liga Alarme //

            } else if (cabecalho.indexOf("GET /al/on") >= 0) { // Liga o Alarme //
              Serial.println("Alarme Sensor ON");
              alarme_estado = "on";
              ativacao = true;


              // Liga Desliga //

            } else if (cabecalho.indexOf("GET /al/off") >= 0) { // Desliga o alarme //
              Serial.println("Alarme Sensor OFF");
              alarme_estado = "off";
              ativacao = false;


              // Resetar o Sistema Esp32 //

            } else if (cabecalho.indexOf("GET /00/off") >= 0) { // Reseta o Esp32 //
              Serial.println("ESP32 Resetado ....");
              ESP.restart();

            } else if (cabecalho.indexOf("GET /02/deletado") >= 0) {
              deletaarquivo(SD, "/alarme.txt");
              deletaarquivo(SD, "/reseta.txt");
              Serial.println("Arquivos Deletado Alarme/Reseta.txt...");


            }  else if (cabecalho.indexOf("GET /02/criado") >= 0) {
              criararquivo(SD, "/alarme.txt", "Grava Disparo do SD. \n");
              criararquivo(SD, "/reseta.txt", "Grava Disparo do SD. \n");
              Serial.println("Arquivos Criado Alarme/Reseta.txt...");


            } else if (cabecalho.indexOf("GET /02/verificado") >= 0) {
              listDir(SD, "/", 0);
              createDir(SD, "/mydir");
              listDir(SD, "/", 0);
              removeDir(SD, "/mydir");
              listDir(SD, "/", 2);
              Serial.println("Pasta de arquivos lidos...");

            }


            // Tela Inicial do HTML via URL //

            client.println("<!DOCTYPE html><html>");

            client.println("<head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"/>");
            client.println("<link rel='shortcut icon' href='https://img.icons8.com/wired/64/000000/touchscreen-smartphone.png'>");   //<link rel="icon" type="image/png" href="img/favicon.png" />



            client.println("<link rel='stylesheet' href='https://use.fontawesome.com/releases/v5.7.2/css/all.css' integrity='sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr' crossorigin='anonymous'>");
            client.println("<style> body {font-family: Arial, Helvetica, sans-serif;} .grid-1 {padding-top: 40px; display: grid; width: 100%; max-width: 750px; margin: 0 auto; grid-template-columns: repeat(3, 1fr); grid-gap: 20px;} .grid-1 div {color: white; padding: 20px;}.item-1 {background: #b03532;}.item-2 {background: #33a8a5;}.item-3 {background: #30997a;}</style>");
            client.println("</head>");

            client.println("<body>");

            client.println("<center>");

            client.println("<h3 style='background-color: #CD5C5C; font-size: 35px; border: none; color: white; padding: 30px 345px; text-align: center;text-decoration: none; display: inline-block; margin: 4px 2px; cursor: pointer; '>Sistema Controle </h3>");

            client.println("<style>.grid-container { display: grid; grid-template-columns: auto auto auto; }.grid-item { text-align: center;  margin: 100px -80px 50px 100px; padding: 10px 10px 0px 0px;  } </style>");



            client.println("<div class='grid-container'>");
            client.println(" <div class='grid-item'>");
            client.println("<p>Lampada 1 - Estado " + rele4estado + "</p>");


            if (rele4estado == "off") {
              client.println("<a style='background-color: #30997a; text-decoration: none; font-size: 100px;   border: none; color: white; padding: 50px 100px; text-align: center; display: inline-block; margin: 4px 2px; cursor: pointer; 'href=\"/4/on\"><i class='far fa-lightbulb'></i></a>");
            } else {
              client.println("<a style='background-color: #b03532; text-decoration: none; font-size: 100px;   border: none; color: white; padding: 50px 100px; text-align: center; display: inline-block; margin: 4px 2px; cursor: pointer; 'href=\"/4/off\"><i class='fas fa-lightbulb'></i></a><br>");
            }
            client.println("</div>");
            // Se o Botao 5 estiver desligado, ele exibe o botão ON

            client.println(" <div class='grid-item'>");
            client.println("<p>Lampada 2 - Estado " + rele15estado + "</p>");

            // Se o Botao 15 estiver desligado, ele exibe o botão ON
            if (rele15estado == "off") {
              client.println("<a style='background-color: #30997a; text-decoration: none; font-size: 100px;   border: none; color: white; padding: 50px 100px; text-align: center; display: inline-block; margin: 4px 2px; cursor: pointer; 'href=\"/15/on\"><i class='far fa-lightbulb'></i></a>");

            } else {
              client.println("<a style='background-color: #b03532; text-decoration: none; font-size: 100px;   border: none; color: white; padding: 50px 100px; text-align: center; display: inline-block; margin: 4px 2px; cursor: pointer; 'href=\"/15/off\"><i class='fas fa-lightbulb'></i></a><br>");
            }

            client.println("</div>");
            client.println("</div>");

            client.println("<div class='grid-container'>");
            client.println(" <div class='grid-item'>");
            client.println("<p>Lampada 1/2 - Estado " + rele4_15estado + "</p>");


            if (rele4_15estado == "off") {
              client.println("<a style='background-color: #30997a; text-decoration: none; font-size: 100px;   border: none; color: white; padding: 50px 100px; text-align: center; display: inline-block; margin: 4px 2px; cursor: pointer; 'href=\"/12/on\"><i class='far fa-bell'></i></a>");
            } else {
              client.println("<a style='background-color: #b03532; text-decoration: none; font-size: 100px;   border: none; color: white; padding: 50px 100px; text-align: center; display: inline-block; margin: 4px 2px; cursor: pointer; 'href=\"/12/off\"><i class='fas fa-bell'></i></a><br>");
            }

            client.println("</div>");

            client.println(" <div class='grid-item'>");
            client.println("<p>Resetar sistema - Estado </p>");
            client.println("<a style='background-color: #b03532; text-decoration: none; font-size: 100px;   border: none; color: white; padding: 50px 100px; text-align: center; display: inline-block; margin: 4px 2px; cursor: pointer; 'href=\"/00/off\"><i class='fas fa-circle-notch'></i></a><br>");

            client.println("</div>");
            client.println("</div>");



            client.println("<div class='grid-container'>");
            client.println(" <div class='grid-item'>");
            client.println("<p>Alarme Estado " + alarme_estado + "</p>");


            if (alarme_estado == "off") {
              client.println("<a style='background-color: #30997a; text-decoration: none; font-size: 100px;   border: none; color: white; padding: 50px 100px; text-align: center; display: inline-block; margin: 4px 2px; cursor: pointer; 'href=\"/al/on\"><i class='far fa-bell'></i></a>");
            } else {
              client.println("<a style='background-color: #b03532; text-decoration: none; font-size: 100px;   border: none; color: white; padding: 50px 100px; text-align: center; display: inline-block; margin: 4px 2px; cursor: pointer; 'href=\"/al/off\"><i class='fas fa-bell'></i></a><br>");
            }

            client.println("</div>");

            client.println("<style>.modal {display: none; position: fixed; z-index: 1; padding-top: 100px;  left: 0; top: 0; width: 100%; height: 100%; background-color: rgb(0,0,0); background-color: rgba(0,0,0,0.4); }</style>");
            client.println("<style>.modal-content { background-color: #fefefe; margin: auto; padding: 20px; border: 1px solid #888; width: 80%; } .close { color: #aaaaaa; float: right; font-size: 28px; font-weight: bold; }</style>");
            client.println("<style>.close:hover, .close:focus { color: #000; text-decoration: none; cursor: pointer; } </style>");

            client.println(" <div class='grid-item'>");
            client.println("<p>SD - Visualizar dados </p>");

            client.println("<a id='bot' style='background-color: #b03532; text-decoration: none; font-size: 100px;   border: none; color: white; padding: 50px 100px; text-align: center; display: inline-block; margin: 4px 2px; cursor: pointer; 'ng-click='myFunc()'><i class='fas fa-sd-card'></i></a><br>");



            client.println("<div id='modelo' class='modal'>");
            client.println("<div class='modal-content'>");
            client.println("<span class='close'>&times;</span>");



            uint64_t tamanhoSD = SD.cardSize() / (1024 * 1024);
            client.printf("<p>Tamanho Total do SD: %lluMB\n", tamanhoSD, "</p>");
            client.printf("<p>Espaco Total: %lluMB\n", SD.totalBytes() / (1024 * 1024), "</p>");
            client.printf("<p>Espaco Usado: %lluMB\n", SD.usedBytes() / (1024 * 1024), "</p>");


            uint8_t cardType = SD.cardType();

            if (cardType == CARD_NONE) {
              client.println("<p>Cartao SD incorreto ou nao aplicado.</p>");
            }
            if (cardType == CARD_MMC) {
              client.println("<p>Tipo de cartao SD: MMC</p>");
            } else if (cardType == CARD_SD) {
              client.println("<p>Tipo de cartao SD: SD_SC</p>");
            } else if (cardType == CARD_SDHC) {
              client.println("<p>Tipo de cartao SD: SD_HC</p>");
            } else {
              client.println("<p>Indefinido...</p>");
            }

            client.println("<p> " + horario + "</p>");
            client.println("<p> " + dia + "</p>");
            client.println("<p> " + datacompleta + "</p>");


            client.println("<p> <a style='background-color: #30997a; text-decoration: none; font-size: 80%;   border: none; color: white; padding: 30px 20% 30px; text-align: center; display: inline-block; margin: 4px 2px; cursor: pointer;' href=\"/02/criado\" >      Criados/</a> </p>");
            client.println("<p> <a style='background-color: #b03532; text-decoration: none; font-size: 80%;   border: none; color: white; padding: 30px 20% 30px; text-align: center; display: inline-block; margin: 4px 2px; cursor: pointer;' href=\"/02/deletado\" >    Deletad/</a> </p>");
            client.println("<p> <a style='background-color: #33a8a5; text-decoration: none; font-size: 80%;   border: none; color: white; padding: 30px 20% 30px; text-align: center; display: inline-block; margin: 4px 2px; cursor: pointer;' href=\"/02/verificado\" > Verific/</a> </p>");

            client.println("</div>");
            client.println("</div>");

            client.println("</div>");
            client.println("</div>");


            client.println("<script> var modal = document.getElementById('modelo'); </script>");
            client.println("<script> var btn = document.getElementById('bot'); </script>");
            client.println("<script> var span = document.getElementsByClassName('close')[0];  </script>");
            client.println("<script> btn.onclick = function() { modal.style.display = 'block'; } </script>");
            client.println("<script> span.onclick = function() { modal.style.display = 'none'; } </script>");
            client.println("<script> window.onclick = function(event) { if (event.target == modal) { </script>");
            client.println("<script> modal.style.display = 'none'; } } </script>");




            // inicia a funcao de Humidade e Temperatura //

            float h = dht.readHumidity();
            float t = dht.readTemperature();
            float f = dht.readTemperature(true);


            if (isnan(h) || isnan(t) || isnan(f)) {
              Serial.println(F("Falha no sensor de Temperatura"));
              return;
            }

            float hif = dht.computeHeatIndex(f, h);
            float hic = dht.computeHeatIndex(t, h, false);

            client.println("<section class='grid-1'>");

            client.println("<div class='item-1'><i style='font-size: 80px;' class='fas fa-tint'></i>");
            client.print(F("HUMIDADE: "));
            client.print(h);
            client.println("%");
            client.println("</div>");

            client.println("<div class='item-2'><i style='font-size: 80px;' class='fas fa-temperature-high'></i>");
            client.print(F(" TEMPERATURA: "));
            client.print(t);
            client.print(F("°C "));
            client.print(f);
            client.println("°F");

            client.println("</div>");

            client.println("<div class='item-3'><i style='font-size: 80px;' class='fas fa-chart-bar'></i>");

            client.print(F("indice de calor: "));
            client.print(hic);
            client.print(F("°C "));
            client.print(hif);
            client.println(F("°F"));
            client.println("</div>");

            client.println("</section>");

            client.println("</center>");
            client.println("</body></html>");


            client.println();     // A resposta HTTP termina com outra linha em branco //


            break;                // Saia do ciclo loop enquanto //
          } else {                // se voce obteve uma nova linha, então limpe currentLine
            linhaatual = "";
          }
        } else if (c != '\r') {   // se você tiver qualquer outra coisa além de um caractere de retorno de trasporte,
          linhaatual += c;        // adicione-o ao final da Linhaatual
        }
      }
    }

    cabecalho = "";                                       // Limpe a variável de cabeçalho

    client.stop();                                        // Fecha a Conexao //
    Serial.println("Cliente Desconectado...");
    Serial.println("");

  } // fim da funcao cliente //
} // end loop //






void Sensor_PIR() {

  bool Dectado = digitalRead(PIR);

  if (Dectado) {

    for (byte i = 0; i < 3; i = i + 1) {                                // valor 3 clientes no array quantidade de ids cadastrados para receber as mensagens do telegran //
      bot.sendSimpleMessage(id[i], "Sua Casa foi Violada porta" , "");   // Mensagens enviada atrves do telegran //


    } // fim do for //

    (!MailClient.sendMail(smtpData));
    smtpData.empty();

    /*
      1800000 Millis por 00:30:00 m
      1200000 Millis por 00:20:00 m
      600000  Millis por 00:10:00 m
      120000  Millis por 00:02:00 m
    */

    unsigned long temposirene = 5000; // mllisegundos 1 S a cada 1000 mil //
    unsigned long novotempo = 0;


    novotempo = millis();
    digitalWrite(Sirene , HIGH);

    sistemadisparado(); // Gravar disparo ao SD... //

    while (millis() < novotempo + temposirene) {

      Serial.println(temposirene);
      Serial.println(novotempo);
      Serial.println("");

    }

    digitalWrite(Sirene , LOW);
    while (millis() < temposirene - novotempo) {
      temposirene = 0;
      novotempo = 0;

    }


  } //end if //

} // end void





// void grava dados do disparo do alarme void sistema disparado //
void sistemadisparado() {
  timeClient.update();
  // Atualizacao infinita no loop //
  unsigned long epochTime = timeClient.getEpochTime();
  String horaalarme = timeClient.getFormattedTime();
  struct tm *ptm = gmtime ((time_t *)&epochTime);
  int monthDay = ptm->tm_mday;
  int currentMonth = ptm->tm_mon + 1;
  int currentYear = ptm->tm_year + 1900;
  String datacompleta = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay);
  escrevedados(SD, "/alarme.txt", "Gravacao - ");
  escrevedados(SD, "/alarme.txt", "Sistema de Alarme Disparado - ");
  escrevedados(SD, "/alarme.txt", horaalarme.c_str());
  escrevedados(SD, "/alarme.txt", " - ");
  escrevedados(SD, "/alarme.txt", datacompleta.c_str());
  escrevedados(SD, "/alarme.txt", "\n");

} // end void //




// grava dados do reset esp32 void resetasistemaesp //
void resetasistemaesp() {
  timeClient.update();
  // Atualizacao infinita no loop //
  unsigned long epochTime = timeClient.getEpochTime();
  String horasreseta = timeClient.getFormattedTime();
  struct tm *ptm = gmtime ((time_t *)&epochTime);
  int monthDay = ptm->tm_mday;
  int currentMonth = ptm->tm_mon + 1;
  int currentYear = ptm->tm_year + 1900;
  String datacompleta = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay);
  escrevedados(SD, "/reseta.txt", "Gravacao - ");
  escrevedados(SD, "/reseta.txt", "Sistema Resetado conforme - ");
  escrevedados(SD, "/reseta.txt", horasreseta.c_str());
  escrevedados(SD, "/reseta.txt", " - ");
  escrevedados(SD, "/reseta.txt", datacompleta.c_str());
  escrevedados(SD, "/reseta.txt", "\n");
}   // end void //


void listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
  Serial.printf("Lista de Arquivos: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("Falha ao abrir o Diretorio...");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("nao existe diretorio...");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}



void createDir(fs::FS &fs, const char * path) {
  Serial.printf("Creating Dir: %s\n", path);
  if (fs.mkdir(path)) {
    Serial.println("Dir created");
  } else {
    Serial.println("mkdir failed");
  }
}



void removeDir(fs::FS &fs, const char * path) {
  Serial.printf("Removing Dir: %s\n", path);
  if (fs.rmdir(path)) {
    Serial.println("Dir removed");
  } else {
    Serial.println("rmdir failed");
  }
}



void readFile(fs::FS &fs, const char * path) {
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while (file.available()) {
    Serial.write(file.read());
  }
  file.close();
}


// cria os arquivos txt para cartao SD //
void criararquivo(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}


// escreve dados no arquivo txt void escrevedados //
void escrevedados(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Anexando ao arquivo...: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Falha ao abrir arquivo para anexar...");
    return;
  }
  if (file.print(message)) {
    Serial.println("Mensagen aplicada ao arquivo");
  } else {
    Serial.println("Falha ao aplicar mensagem no arquivo");
  }
  file.close();
}



void renameFile(fs::FS &fs, const char * path1, const char * path2) {
  Serial.printf("Renaming file %s to %s\n", path1, path2);
  if (fs.rename(path1, path2)) {
    Serial.println("File renamed");
  } else {
    Serial.println("Rename failed");
  }
}






// Delete os arquivos do SD void deletararquivo//

void deletaarquivo(fs::FS &fs, const char * path) {
  Serial.printf("Deleting file: %s\n", path);
  if (fs.remove(path)) {
    Serial.println("Arquivos Deletados...");
  } else {
    Serial.println("Falha a deletar arquivos...");
  }
}

// end void //



void Ledgreen() {

  digitalWrite(Green , HIGH);
  delay(200);
  digitalWrite(Green , LOW);
  delay(200);
}

void LedVermelho() {

  digitalWrite(Red , HIGH);
  delay(200);
  digitalWrite(Red , LOW);
  delay(200);
}

void sendCallback(SendStatus msg) {
  Serial.println(msg.info());
}
