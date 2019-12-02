
#define DEBUG		0

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <map>
#include <string>
#include <vector>
#include <algorithm>		
#include <stack>
using namespace std;

struct seq_tag 
{
	const char *sequence;
	uint8_t padding;
	uint8_t id;
};

typedef struct seq_tag seq_t;

/* Sequence IDs */
enum seq_id {
	VK_UP,
	VK_DOWN,
	VK_LEFT,
	VK_RIGHT,
	VK_INSERT,
	VK_HOME,
	VK_PGUP,
	VK_DELETE,
	VK_END,
	VK_PGDN,
	VK_F1,
	VK_F2,
	VK_F3,
	VK_F4,
	VK_F5,
	VK_F6,
	VK_F7,
	VK_F8,
	VK_F9,
	VK_F10,
	VK_F11,
	VK_F12
};

/* Sequence IDs and matching sequence */
seq_t edata[] =
{
	{"\x1B\x5B\x41", 		 3, VK_UP},
	{"\x1B\x5B\x42", 		 3, VK_DOWN},
	{"\x1B\x5B\x44", 		 3, VK_LEFT},
	{"\x1B\x5B\x43", 		 3, VK_RIGHT},
	{"\x1B\x5B\x31\x7E", 	 4, VK_INSERT},
	{"\x1B\x5B\x32\x7E", 	 4, VK_HOME},
	{"\x1B\x5B\x33\x7E", 	 4, VK_PGUP},
	{"\x1B\x5B\x34\x7E", 	 4, VK_DELETE},
	{"\x1B\x5B\x35\x7E", 	 4, VK_END},
	{"\x1B\x5B\x36\x7E", 	 4, VK_PGDN},
	{"\x1B\x5B\x31\x31\x7E", 5, VK_F1},
	{"\x1B\x5B\x31\x32\x7E", 5, VK_F2},
	{"\x1B\x5B\x31\x33\x7E", 5, VK_F3},
	{"\x1B\x5B\x31\x34\x7E", 5, VK_F4},
	{"\x1B\x5B\x31\x35\x7E", 5, VK_F5},
	{"\x1B\x5B\x31\x37\x7E", 5, VK_F6},
	{"\x1B\x5B\x31\x38\x7E", 5, VK_F7},
	{"\x1B\x5B\x31\x39\x7E", 5, VK_F8},
	{"\x1B\x5B\x32\x30\x7E", 5, VK_F9},
	{"\x1B\x5B\x32\x31\x7E", 5, VK_F10},
	{"\x1B\x5B\x32\x33\x7E", 5, VK_F11},
	{"\x1B\x5B\x32\x34\x7E", 5, VK_F12},	
	{NULL, 0, 0}
};

class node
{
public:
	int id;
	uint32_t value;
	bool terminal;
	map<char, node*> hash;

	static int class_id;

	node()
	{
		id = class_id++;
		value = -1;
		terminal = false;
	}

	node(uint32_t value, bool is_terminal = false)
	{
		id = class_id++;
		this->value = value;
		terminal = is_terminal;
	}
};

int node::class_id = 0;



class escape_parser
{
public:
	node *root;
	vector<node*> nodelist;
	string seq;
	FILE *fd;


	escape_parser(void)
	{
		root =  new node();
		nodelist.push_back(root);
	}



	bool insert(string sequence, uint32_t value)
	{
		node *cursor = root;
		std::reverse(sequence.begin(), sequence.end());

		for(int i = 0; i < sequence.size(); i++)
		{
			char ch = sequence[i];
			bool last = (i == sequence.size() - 1);

			auto it = cursor->hash.find(ch);

			if(it == cursor->hash.end())
			{
				node *temp = new node(last ? value : 0, last);
				nodelist.push_back(temp);
				cursor->hash[ch] = temp;
			}

			if(!last)
			{
				cursor = cursor->hash[ch];
			}
		}
		return true;
	} /* insert */




	void print_helper(node *cursor)
	{
		for(auto it = cursor->hash.begin(); it != cursor->hash.end(); it++)
		{
			seq += it->first;
			print_helper(it->second);

			if(it->second->terminal == true)
			{
				string temp = seq;
				std::reverse(temp.begin(), temp.end());
				printf("Sequence: ");
				for(int i = 0; i < temp.size(); i++)
				{
					printf("[%02X] ", temp[i]);
				}
				printf("\nHas value %008X\n", it->second->value);
			}
			seq.pop_back();

		}
	}

	void print_sequences(void)
	{
		print_helper(root);
	}

	void list_nodes(void)
	{
		for(int i = 0; i < nodelist.size(); i++)
		{
			printf("Node HashSize=%02d Terminal=%02d, Value=%08X, Id=%02d \n",
				nodelist[i]->hash.size(),
				nodelist[i]->terminal,
				nodelist[i]->value,
				nodelist[i]->id
				);
		}
	}

	void generate_parser(FILE *fd);
	void generate_helper(node *cursor);



	void emit(const char *fmt, ...)
	{
		va_list ap;
		fprintf(fd, "\t");
		va_start(ap, fmt);
		vfprintf(fd, fmt, ap);
		va_end(ap);
		fprintf(fd, "\r\n");
	}
	void emit_label(const char *fmt, ...)
	{
		va_list ap;
		va_start(ap, fmt);
		vfprintf(fd, fmt, ap);
		va_end(ap);
		fprintf(fd, "\r\n");
	}
	void emit_comment(const char *fmt, ...)
	{
		va_list ap;
		va_start(ap, fmt);
		vfprintf(fd, fmt, ap);
		va_end(ap);
		fprintf(fd, "\r\n");
	}


	uint32_t find(string s)
	{

		node *cursor = root;
		for(int i = s.size()-1; i >= 0; i--)
		{
			char ch = s[i];

			if(cursor->hash.count(ch))
			{
				cursor = cursor->hash[ch];
				if(i == 0 && cursor->terminal)
				{
					return cursor->value;
				}

			}
			else
			{
				return -1; // bail
			}
		}
		return -1;
	}
};




void escape_parser::generate_helper(node *cursor)
{
	int source_areg = 0;
	int output_dreg = 0;
	int value_dreg = 1;
	int base_areg = 1;

#if DEBUG
	emit_comment("# Node Id=%02X Value=%08X Size=%d Terminal:%d",
			cursor->id,
			cursor->value,
			cursor->hash.size(),
			cursor->terminal
			);
#endif
	/* Emit label for this matching sequence */
	emit_label("MapMatch%04X:", cursor->id);

	/* If this is an empty node, emit terminal value and exit */
	if(cursor->terminal)
	{
		emit("\tmoveq\t#$%02X, d%d",
			cursor->value,
			output_dreg
			);
		emit("\trts");
		return;
	}

	/* Get a new character */
	emit("\tcmpa\ta%d, a%d", source_areg, base_areg);
	emit("\tbeq.w\t.abort");
	emit("\tmove.b\t-(a%d), d%d", source_areg, output_dreg);

	/* Emit parse code */
	for(auto it = cursor->hash.begin(); it != cursor->hash.end(); it++)
	{
		emit("\tcmpi.b\t#$%02X, d%d", 
			it->first,
			output_dreg
			);
		emit("\tbeq.w\tMapMatch%04X", it->second->id);
	}

	/* No match, exit */
	emit("\tbra.w\t.abort");

	/* Process next entries */
	for(auto it = cursor->hash.begin(); it != cursor->hash.end(); it++)
	{
		generate_helper(it->second);
	}
}



void escape_parser::generate_parser(FILE *fd)
{
	int source_areg = 0;
	int output_dreg = 0;
	int value_dreg = 1;
	int base_areg = 1;

	this->fd = fd;

	emit_comment("# This is an generated file. Do not edit.");


	emit(".abort:");
	emit("\trts");
	emit("escape_parser:");
	emit("\tmoveq\t#-1, d%d", value_dreg);

	generate_helper(root);

	fprintf(fd, "/* End */");
}



int main (int argc, char *argv[])
{
	escape_parser parser;
#if 1
	for(int i = 0; edata[i].sequence != NULL; i++)
	{
		string temp = edata[i].sequence;
		std::reverse(temp.begin(), temp.end());
		parser.insert(temp, edata[i].id);
	}
#else
	parser.insert("ABCA", 0xaa);
	parser.insert("ABCB", 0xaa);
	parser.insert("ABCC", 0xaa);
	parser.insert("ABCX", 0x55);

	int status = parser.find("ABCC");
	printf("find result = %d\n", status);
	status = parser.find("ABCY");
	printf("find result = %d\n", status);
	status = parser.find("DEFE");
	printf("find result = %d\n", status);
#endif	
	parser.list_nodes();
	parser.print_sequences();

	for(int i = 0; edata[i].sequence != NULL; i++)
	{
		if(parser.find(edata[i].sequence) != -1)
		{
			printf("Error: Couldn't find sequence %d\n", i);
		}
	}

	FILE *fd;
	fd = fopen("esc_parse.s", "wb");
	parser.generate_parser(fd);
	fclose(fd);

	return 0;
}

/* End */