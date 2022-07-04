#include <EEPROM.h>
#include <Hash.h>


class Multi_Half_Duplex_UART
{
  public:
    int command;
    String data;
    int UART_EEPROM_ADDRESS = 250;
    int UART_ADDRESS = EEPROM.read(UART_EEPROM_ADDRESS);
    int baudrate;

//    Sender Vars
    String message_id;
    String message_to_send;
    String message_hex;
    String message_hash;
    int reciever_id;

    bool init_sending=false;
    bool start_sending=false;
    bool end_sending=false;


//    Reciever Vars
    String message_id_recieving;
    String message_recieved;
    String message_id_recieved;
   String message_hex_recieved;
    String message_hash_recieved;

    bool waiting_to_send=false;
    bool start_recieving=false;
    bool end_recieving=false;








    Multi_Half_Duplex_UART(int tempbaudrate)
    {
      baudrate = tempbaudrate;
    }

void setup_init_message(int reciever_id_temp, String message_to_send_temp) {
          reciever_id = reciever_id_temp;
          message_to_send=message_to_send_temp;
        message_id = String(random(32000));
      message_hex = stringToHex(message_to_send);
      message_hash = calculate_hash(message_hex);
}

    void init_send_message_now() {

     init_sending=true;
     start_sending=false;
     end_sending=false;
     Serial.println("2>" + String(UART_ADDRESS) + "|" + String(reciever_id) + "|" + String(message_id) + "|");
    }

    

    

    void intialize_sending(int reciever_id_temp, String message_to_send_temp) {

      if(start_recieving){
        if(not waiting_to_send){
          waiting_to_send=true;
           setup_init_message(reciever_id_temp, message_to_send_temp);
        }

      }else{

        setup_init_message(reciever_id_temp, message_to_send_temp);
        init_send_message_now();
      }

    }

    String calculate_hash(String msg_hash) {
      return sha1(msg_hash);
    }


    void change_UART_ADDRESS(int value) {
      UART_ADDRESS = value;
      EEPROM.write(UART_EEPROM_ADDRESS, value);
    }

    void setup() {

      Serial.begin(baudrate);
      Serial.println(UART_ADDRESS);

    }


    String stringToHex(String input) {
      String hex = "";
      for (int i = 0 ; i < input.length(); i++) {
        char c = (char)input[i];
        hex += String(c, HEX);
      }
      return hex;
    }

    String hexToString(String hex)
    {
      String text = "";
      for (int k = 0; k < hex.length(); k++)
      {
        if (k % 2 != 0)
        {
          char temp[3];
          sprintf(temp, "%c%c", hex[k - 1], hex[k]);
          int number = (int)strtol(temp, NULL, 16);
          text += char(number);
        }
      }
      return text;
    }


    String separate_string(String data, char separator, int index)
    {
      int found = 0;
      int strIndex[] = {0, -1};
      int maxIndex = data.length() - 1;

      for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
          found++;
          strIndex[0] = strIndex[1] + 1;
          strIndex[1] = (i == maxIndex) ? i + 1 : i;
        }
      }

      return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
    }


    void get_data(String payload) {
      data = separate_string(payload, '>', 1);
    }

    void get_command(String payload) {
      command = separate_string(payload, '>', 0).toInt();
    }


    int get_int_data(int index = 0) {
      return separate_string(data, '|', index).toInt();
    }


    String get_string_data(int index = 0) {
      return separate_string(data, '|', index);
    }





    void loop() {
      if (Serial.available())
      {

        static String payload;



        payload = Serial.readStringUntil('\n');


        main_loop(payload);

      }
    }


    bool main_loop( String payload) {


      get_command(payload);
      get_data(payload);




      ////////////////////////////////////////////////////
      //    Serial Commands
      ////////////////////////////////////////////////////

      int sender = get_int_data(0);
      int receiver = get_int_data(1);



      if (UART_ADDRESS == receiver) {


        switch (command)
        {

          // change uart address
          //        1>|255|20
          case 1:
            change_UART_ADDRESS(get_int_data(2));
            break;


          // init sending
          // sender --> reciever
          case 2:
            // send ack to sender back
            message_id_recieving = get_int_data(2);
 
     start_recieving=true;
     end_recieving=false;

            Serial.println("3>" + String(UART_ADDRESS) + "|" + String(sender) + "|" + String(message_id_recieving) + "|");
            break;


          // ack send
          // reciever --> sender
          case 3:

            Serial.println("4>" + String(UART_ADDRESS) + "|" + String(sender) + "|" + String(message_id) + "|" + String(message_hex) + "|" + String(message_hash) + "|");
     init_sending=true;
     start_sending=true;
     end_sending=false;
            break;


          // start sending
          // sender --> reciever
          case 4:
             message_id_recieved = get_string_data(2);
             message_hex_recieved = get_string_data(3);
             message_hash_recieved = get_string_data(4);

            if (calculate_hash(message_hex_recieved) == message_hash_recieved) {
              //            correct data recieved
              Serial.println("6>" + String(UART_ADDRESS) + "|" + String(sender) + "|" + String(message_id_recieving) + "|");
              message_recieved = hexToString(message_hex_recieved);
              Serial.println(message_recieved);
     start_recieving=false;
     end_recieving=true;
     if(waiting_to_send){
      waiting_to_send=false;
      init_send_message_now();
     }
     
            } else {
              Serial.println("5>" + String(UART_ADDRESS) + "|" + String(sender) + "|" + String(message_id_recieving) + "|");
            }

            break;

          // start resend
          // reciever --> sender
          case 5:
            Serial.println("4>" + String(UART_ADDRESS) + "|" + String(sender) + "|" + String(message_id) + "|" + String(message_hex) + "|" + String(message_hash) + "|");
     init_sending=true;
     start_sending=true;
     end_sending=false;
            break;
            

          // completed sending
          // reciever --> sender
          case 6:
     init_sending=false;
     start_sending=false;
     end_sending=true;
            break;


          // send message
          // computer --> sender --> reciever
          case 7:
             intialize_sending(sender, get_string_data(2));
            break;





          default:
            // code block

            Serial.println("1>0|");
            return false;
        }

      }


      return true;


    }

};
