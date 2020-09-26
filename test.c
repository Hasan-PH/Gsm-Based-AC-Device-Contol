#define F_CPU 1000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#define SREG _SFR_IO8(0x3F)
#define bulb1 0
#define bulb2 1
#define bulb3 2
#define bulb4 3
#define on 1
#define off 0

void GSM_Begin();
void GSM_Calling(char *);
void GSM_HangCall();
void GSM_Response();
void GSM_Response_Display();
void GSM_Msg_Read(int);
bool GSM_Wait_for_Msg();
void GSM_Msg_Display();
void GSM_Msg_Delete(unsigned int);
void GSM_Send_Msg(char *, char *);
void GSM_Delete_All_Msg();

char buff[160];		  /* buffer to store responses and messages */
char status_flag = 0; /* for checking any new message */
volatile int buffer_pointer;
char Mobile_no[] = "+8801517800153"; /* store mobile no. of received message */ //Sir +8801912349704
char message_received[60];														/* save received message */
int position = 0;																/* save location of current message */
char mmm[] = "I do not hear you";
int device_no;
int logic_state;
char con[30];

void USART_Init(unsigned long BAUDRATE) /* USART initialize function */
{
	int BAUD_PRESCALE = (((F_CPU / (BAUDRATE * 16UL))) - 1);
	UCSRB |= (1 << RXEN) | (1 << TXEN) | (1 << RXCIE);	 /* Enable USART transmitter and receiver */
	UCSRC |= (1 << URSEL) | (1 << UCSZ0) | (1 << UCSZ1); /* Write USCRC for 8 bit data and 1 stop bit */
	UBRRL = BAUD_PRESCALE;								 /* Load UBRRL with lower 8 bit of prescale value */
	UBRRH = (BAUD_PRESCALE >> 8);						 /* Load UBRRH with upper 8 bit of prescale value */
}

char USART_RxChar() /* Data receiving function */
{
	while (!(UCSRA & (1 << RXC)))
		;		  /* Wait until new data receive */
	return (UDR); /* Get and return received data */
}

void USART_TxChar(char data) /* Data transmitting function */
{
	UDR = data; /* Write data to be transmitting in UDR */
	while (!(UCSRA & (1 << UDRE)))
		; /* Wait until data transmit and buffer get empty */
}

void USART_SendString(char *str) /* Send string of USART data function */
{
	int i = 0;
	while (str[i] != 0)
	{
		USART_TxChar(str[i]); /* Send each char of string till the NULL */
		i++;
	}
}

ISR(USART_RXC_vect)
{
	while (!(UCSRA & (1 << RXC)))
		;
	buff[buffer_pointer] = UDR;

	buffer_pointer++;
	status_flag = 1;
}

void usart_rx_reset()
{
	UCSRB &= ~(1 << RXCIE); // Disable RX interrupt
	buffer_pointer = 0;		// Reset variables
	buff[buffer_pointer] = '\0';
}

void usart_rx_off()
{
	UCSRB &= ~(1 << RXCIE); // Disable RX interrupt
}

void usart_rx_on()
{
	UCSRB |= (1 << RXCIE);
}

void send_error(char *error)
{
	//printf("@@%s@@\n",error);
	strcat(con, error);
	PORTA &= 0b11101111;
}

void action(char *mess)
{
	const char delim[2] = ":";
	char device[10];
	char state[4];
	memset(device, '\0', sizeof(device));
	memset(state, '\0', sizeof(state));
	/* get the first command */
	int i = 0, j = 0;
	int delim_recieved = 0;
	while (mess[i] != '\0')
	{
		if (mess[i] == ':')
		{
			delim_recieved = 1;
			j = -1;
		}
		else if (!delim_recieved)
		{
			device[j] = tolower(mess[i]);
		}
		else
		{
			state[j] = tolower(mess[i]);
		}
		i++;
		j++;
	}
	//printf( " %s-->%s\n", device, state );
	if (!strcmp(state, "on"))
	{
		//logic_state = 1;
		if (!strcmp(device, "b1"))
		{
			//device_no = bulb1;
			if (PORTA & 0b00000001)
			{
				PORTA &= 0b11111110;
				PORTA |= 0b00010000;
				strcat(con, "b1:on ");
			}
			else
			{
				send_error("b1:alr on ");
			}
		}
		else if (!strcmp(device, "b2"))
		{
			device_no = bulb2;
			if (PORTA & 0b00000010)
			{
				PORTA &= 0b11111101;
				PORTA |= 0b00010000;
				strcat(con, "b2:on ");
			}
			else
			{
				send_error("b2:alr on ");
			}
		}
		else if (!strcmp(device, "b3"))
		{
			device_no = bulb3;
			if (PORTA & 0b00000100)
			{
				PORTA &= 0b11111011;
				PORTA |= 0b00010000;
				strcat(con, "b3:on ");
			}
			else
			{
				send_error("b3:alr on ");
			}
		}
		else if (!strcmp(device, "b4"))
		{
			device_no = bulb4;
			if (PORTA & 0b00001000)
			{
				PORTA &= 0b11110111;
				PORTA |= 0b00010000;
				strcat(con, "b4:on ");
			}
			else
			{
				send_error("b4:alr on ");
			}
		}
		else
		{
			send_error("invalid_device_name");
		}
	}
	else if (!strcmp(state, "off"))
	{
		logic_state = 0;

		if (!strcmp(device, "b1"))
		{

			//device_no = bulb1;

			if (PORTA & 0b00000001)
			{
				send_error("b1:alr off ");
			}
			else
			{

				PORTA |= 0b00000001;
				PORTA |= 0b00010000;
				strcat(con, "b1:off ");
			}
		}
		else if (!strcmp(device, "b2"))
		{
			//device_no = bulb2;
			if (PORTA & 0b00000010)
			{
				send_error("b2:alr off ");
			}
			else
			{

				PORTA |= 0b00000010;
				PORTA |= 0b00010000;
				strcat(con, "b2:off ");
			}
		}
		else if (!strcmp(device, "b3"))
		{
			device_no = bulb3;
			if (PORTA & 0b00000100)
			{
				send_error("b3:alr off ");
			}
			else
			{
				PORTA |= 0b00000100;
				PORTA |= 0b00010000;
				strcat(con, "b3:off ");
			}
		}
		else if (!strcmp(device, "b4"))
		{
			device_no = bulb4;
			if (PORTA & 0b00001000)
			{
				send_error("b4:alr off ");
			}
			else
			{
				PORTA |= 0b00001000;
				PORTA |= 0b00010000;
				strcat(con, "b4:off ");
			}
		}
		else
		{
			send_error("invalid_device_name");
		}
	}
	else
	{
		send_error("Invalid state.");
	}

	//PORTA |= (logic_state << device_no);
}

void analysis()
{
	//char message_recieved[80] = "Bulb1:off,Bulb2:on,Bulb3:off,Bulb4:on";
	const char delim[2] = ",";
	char *command;
	/* get the first command */
	command = strtok(message_received, delim);

	while (command != NULL)
	{
		//printf( " %s\n", command );
		action(command);
		command = strtok(NULL, delim);
	}
	//PORTA=0b11111010;
	strcat(con, "\0");
}

////////////////////////////////////////////////////////////////////////////////////

int main(void)
{
	DDRA = 0xFF;
	PORTA = 0xFF;
	//PORTA=0b11111110;
	buffer_pointer = 0;
	int is_msg_arrived;
	memset(message_received, 0, 60);

	USART_Init(9600);

	sei();
	_delay_ms(500);
	//GSM_Begin();
	//GSM_Delete_All_Msg();
	while (1)
	{
		//PORTA=0xFF;
		if (status_flag == 1)
		{

			is_msg_arrived = GSM_Wait_for_Msg(); /*check for message arrival*/
			if (is_msg_arrived == true)
			{
				//PORTA=0xFF;
				GSM_Msg_Read(position);
				_delay_ms(1000);

				if (strstr(Mobile_no, "+8801517800153"))
				{

					analysis();
					_delay_ms(2000);
					GSM_Send_Msg(Mobile_no, con);
				}
				else
				{
				}
				//PORTA=0b11111110;

				_delay_ms(2000);
				//
			}

			is_msg_arrived = 0;
			status_flag = 0;
		}
		memset(con, 0, 30);
		memset(Mobile_no, 0, 14);
		memset(message_received, 0, 60);
	}
}

void GSM_Begin()
{

	while (1)
	{

		USART_SendString("ATE0\r");
		_delay_ms(500);
		if (strstr(buff, "OK"))

		{

			GSM_Response();
			memset(buff, 0, 160);
			PORTA = 0x00;
			break;
		}
		else
		{
		}
	}
	status_flag = 0;
}

void GSM_Msg_Delete(unsigned int position)
{
	buffer_pointer = 0;
	char delete_cmd[20];
	sprintf(delete_cmd, "AT+CMGD=%d\r", position); /* delete message at specified position */
	USART_SendString(delete_cmd);
}

void GSM_Delete_All_Msg()
{
	USART_SendString("AT+CMGDA=\"DEL ALL\"\r"); /* delete all messages of SIM */
}

bool GSM_Wait_for_Msg()
{
	char msg_location[4];
	int i;
	_delay_ms(500);
	buffer_pointer = 0;

	while (1)
	{
		if (buff[buffer_pointer] == '\r' || buff[buffer_pointer] == '\n') /*eliminate "\r \n" which is start of string */
		{
			buffer_pointer++;
		}
		else
			break;
	}

	if (strstr(buff, "CMTI:")) /* "CMTI:" to check if any new message received */
	{
		while (buff[buffer_pointer] != ',')
		{
			buffer_pointer++;
		}
		buffer_pointer++;

		i = 0;
		while (buff[buffer_pointer] != '\r')
		{
			msg_location[i] = buff[buffer_pointer]; /* copy location of received message where it is stored */
			buffer_pointer++;
			i++;
		}

		/* convert string of position to integer value */
		position = atoi(msg_location);

		memset(buff, 0, strlen(buff));
		//memset(msg_location,0,strlen(msg_location));
		buffer_pointer = 0;

		return true;
	}
	else
	{
		return false;
	}
}

void GSM_Send_Msg(char *num, char *sms)
{
	char sms_buffer[35];
	buffer_pointer = 0;
	sprintf(sms_buffer, "AT+CMGS=\"%s\"\r", num);

	USART_SendString(sms_buffer); /*send command AT+CMGS="Mobile No."\r */
	_delay_ms(200);
	while (1)
	{
		if (buff[buffer_pointer] == 0x3e) /* wait for '>' character*/
		{
			buffer_pointer = 0;
			memset(buff, 0, strlen(buff));
			USART_SendString(sms); /* send msg to given no. */
			USART_TxChar(0x1a);	   /* send Ctrl+Z then only message will transmit*/
			break;
		}
		buffer_pointer++;
	}
	_delay_ms(300);
	buffer_pointer = 0;
	memset(buff, 0, strlen(buff));
	memset(sms_buffer, 0, strlen(sms_buffer));
}

void GSM_Calling(char *Mob_no)
{
	char call[19];
	sprintf(call, "ATD%s;\r", Mob_no);
	USART_SendString(call); /* send command ATD<Mobile_No>; for calling*/
}

void GSM_HangCall()
{
	USART_SendString("ATH\r"); /*send command ATH\r to hang call*/
}

void GSM_Response()
{
	unsigned int timeout = 0;
	int CRLF_Found = 0;
	char CRLF_buff[2];
	int Response_Length = 0;
	while (1)
	{
		if (timeout >= 60000) /*if timeout occur then return */
			return;
		Response_Length = strlen(buff);
		if (Response_Length)
		{
			_delay_ms(2);
			timeout++;
			if (Response_Length == strlen(buff))
			{
				int i = 0;
				for (i = 0; i < Response_Length; i++)
				{
					memmove(CRLF_buff, CRLF_buff + 1, 1);
					CRLF_buff[1] = buff[i];
					if (strncmp(CRLF_buff, "\r\n", 2))
					{
						if (CRLF_Found++ == 2) /* search for \r\n in string */
						{
							//GSM_Response_Display();		/* display response */
							return;
						}
					}
				}
				CRLF_Found = 0;
			}
		}
		_delay_ms(1);
		timeout++;
	}
	status_flag = 0;
}

void GSM_Msg_Read(int position)
{
	char read_cmd[11];
	sprintf(read_cmd, "AT+CMGR=%d\r", position);
	USART_SendString(read_cmd);
	_delay_ms(3000);
	GSM_Msg_Display();
}

void GSM_Msg_Display()
{
	// if(!(strstr(buff,"+CMGR")))
	// {

	// }

	if ((strstr(buff, "+CMGR")))
	{

		buffer_pointer = 0;

		while (1)
		{
			if (buff[buffer_pointer] == '\r' || buff[buffer_pointer] == 'n')
			{
				buffer_pointer++;
			}
			else
				break;
		}

		/* search for 1st ',' to get mobile no.*/
		while (buff[buffer_pointer] != ',')
		{
			buffer_pointer++;
		}
		buffer_pointer = buffer_pointer + 2;

		/* extract mobile no. of message sender */
		int i = 0;
		for (i = 0; i <= 13; i++)
		{
			Mobile_no[i] = buff[buffer_pointer];
			buffer_pointer++;
		}

		do
		{
			buffer_pointer++;
		} while (buff[buffer_pointer - 1] != '\n');

		i = 0;

		while (buff[buffer_pointer] != '\r' && i < 31)
		{
			message_received[i] = buff[buffer_pointer];

			buffer_pointer++;
			i++;
		}

		buffer_pointer = 0;
		memset(buff, 0, strlen(buff));
	}
	status_flag = 0;
}