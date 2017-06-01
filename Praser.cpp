#include"Praser.h"
#include"block.h"
#include "tools.h"
#include<map>
using namespace std;

Praser::Praser() {
	praserInit();
}

Praser::~Praser() {
	//print_map();
	print_code();
}

void Praser::praserInit() {
	Block wholeBlock;
	blockStack.push_back(wholeBlock);

	build_in_function.insert("read");
	build_in_function.insert("write");

	praserGramTree(root);		//开始分析语法树
}

void Praser::praserGramTree(struct gramTree* node) {
	//cout << "at " << node->name << endl;
	if (node == NULL || node->line == -1)
		return;

	if (node->name == "declaration") {
		node = praser_declaration(node);
	}
	else if (node->name == "function_definition") {
		node = praser_function_definition(node);
	}
	else if (node->name == "statement") {
		node = praser_statement(node);
	}

	//继续向下分析
	if (node != NULL) {
		praserGramTree(node->left);
		praserGramTree(node->right);
	}
}

struct gramTree* Praser::praser_statement(struct gramTree* node) {
	struct gramTree* next = node->left;
	if (node->left->name == "labeled_statement") {

	}
	if (node->left->name == "compound_statement") {
		praser_compound_statement(node->left);
	}
	if (node->left->name == "expression_statement") {
		praser_expression_statement(node->left);
	}
	if (node->left->name == "selection_statement") {
		praser_selection_statement(node->left);
	}
	if (node->left->name == "iteration_statement") {
		praser_iteration_statement(node->left);
	}
	if (node->left->name == "jump_statement") {
		praser_jump_statement(node->left);
	}

	return node->right;
}

void Praser::praser_jump_statement(struct gramTree* node) {
	if (node->left->name == "GOTO") {

	}
	else if (node->left->name == "CONTINUE") {

	}
	else if (node->left->name == "BREAK") {

	}
	else if (node->left->name == "RETURN") {
		string funcType = getFuncRType();
		if (node->left->right->name == "expression") {//return expression
			varNode rnode = praser_expression(node->left->right);
			innerCode.addCode(innerCode.createCodeforReturn(rnode));
			if (rnode.type != funcType) {
				error(node->left->right->line, "return type doesn't equal to function return type.");
			}
		}
		else if (node->left->right->name == ";"){//return ;
			innerCode.addCode("RETURN");
			if (funcType != "void") {
				error(node->left->right->line, "You should return " + blockStack.back().func.rtype);
			}
		}
	}
}

void Praser::praser_expression_statement(struct gramTree *node) {
	if (node->left->name == "expression") {
		praser_expression(node->left);
	}
}

varNode Praser::praser_expression(struct gramTree* node) {
	if (node->left->name == "expression") {
		return praser_expression(node->left);
	}
	else if (node->left->name == "assignment_expression") {
		return praser_assignment_expression(node->left);
	}
	if (node->right->name == ",") {
		return praser_assignment_expression(node->right->right);
	}
}

void Praser::praser_compound_statement(struct gramTree* node) {
	//继续分析处理compound_statement
	praserGramTree(node);
}

//if else 和 switch
void Praser::praser_selection_statement(struct gramTree* node) {


	if (node->left->name == "IF") {
		if (node->left->right->right->right->right->right == NULL) {
			//添加一个新的block
			Block newblock;
			blockStack.push_back(newblock);

			gramTree* expression = node->left->right->right;
			varNode exp_rnode = praser_expression(expression);
			gramTree* statement = node->left->right->right->right->right;

			string label1 = innerCode.getLabelName();
			string label2 = innerCode.getLabelName();
			innerCode.addCode("IF " + innerCode.getNodeName(exp_rnode) + " != " + " #0 GOTO " + label1);
			innerCode.addCode("GOTO " + label2);
			innerCode.addCode(label1 + " :");


			praser_statement(statement);
			
			innerCode.addCode(label2 + " :");

			//弹出添加的block
			blockStack.pop_back();

		}
		else if (node->left->right->right->right->right->right->name == "ELSE") {
			//添加一个新的block
			Block newblock1;
			blockStack.push_back(newblock1);

			gramTree* expression = node->left->right->right;
			varNode exp_rnode = praser_expression(expression);
			gramTree* statement1 = node->left->right->right->right->right;
			gramTree* statement2 = node->left->right->right->right->right->right->right;

			string label1 = innerCode.getLabelName();
			string label2 = innerCode.getLabelName();
			string label3 = innerCode.getLabelName();
			innerCode.addCode("IF " + innerCode.getNodeName(exp_rnode) + " != " + " #0 GOTO " + label1);
			innerCode.addCode("GOTO " + label2);
			innerCode.addCode(label1 + " :");

			praser_statement(statement1);
			
			innerCode.addCode("GOTO " + label3);
			//弹出添加的block
			blockStack.pop_back();

			//else
			innerCode.addCode(label2 + " :");

			Block newblock2;
			blockStack.push_back(newblock2);

			praser_statement(statement2);

			innerCode.addCode(label3 + " :");

			//弹出添加的block
			blockStack.pop_back();

		}
	}
	else if (node->left->name == "SWITCH") {

	}
	
}

//循环 while for do while
void Praser::praser_iteration_statement(struct gramTree* node) {
	if (node->left->name == "WHILE") {

		//添加一个新的block
		Block newblock;
		blockStack.push_back(newblock);

		struct gramTree* expression = node->left->right->right;
		struct gramTree* statement = node->left->right->right->right->right;

		string label1 = innerCode.getLabelName();
		string label2 = innerCode.getLabelName();
		string label3 = innerCode.getLabelName();

		innerCode.addCode(label1 + " :");

		varNode var = praser_expression(expression);

		innerCode.addCode("IF " + innerCode.getNodeName(var) + " != " + "#0 GOTO " + label2);
		innerCode.addCode("GOTO " + label3);
		innerCode.addCode(label2 + " :");

		praser_statement(statement);

		innerCode.addCode("GOTO " + label1);
		innerCode.addCode(label3 + " :");

		//弹出添加的block
		blockStack.pop_back();
	}
	else if (node->left->name == "DO") {
		//添加一个新的block
		Block newblock;
		blockStack.push_back(newblock);

		struct gramTree* statement = node->left->right;
		struct gramTree* expression = node->left->right->right->right->right;

		string label1 = innerCode.getLabelName();
		string label2 = innerCode.getLabelName();

		innerCode.addCode(label1 + " :");

		praser_statement(statement);

		varNode var = praser_expression(expression);

		innerCode.addCode("IF " + innerCode.getNodeName(var) + " == " + "#0 GOTO " + label2);
		innerCode.addCode("GOTO " + label1);
		innerCode.addCode(label2 + " :");

		//弹出添加的block
		blockStack.pop_back();

	}
	else if (node->left->name == "FOR") {
		if (node->left->right->right->name == "expression_statement") {
			//FOR '(' expression_statement expression_statement ')'statement
			if (node->left->right->right->right->right->name == ")") {
				//添加一个新的block
				Block newblock;
				blockStack.push_back(newblock);

				gramTree* exp_state1 = node->left->right->right;
				gramTree* exp_state2 = exp_state1->right;
				gramTree* statement = exp_state2->right->right;

				string label1 = innerCode.getLabelName();
				string label2 = innerCode.getLabelName();
				string label3 = innerCode.getLabelName();

				if (exp_state1->left->name == "expression") {
					praser_expression(exp_state1->left);
				}
				innerCode.addCode(label1 + " :");

				varNode var;
				if (exp_state2->left->name == "expression") {
					var = praser_expression(exp_state2->left);
					innerCode.addCode("IF " + innerCode.getNodeName(var) + " != " + "#0 GOTO " + label2);
				}
				else {
					innerCode.addCode("GOTO " + label2);
				}

				innerCode.addCode("GOTO " + label3);
				innerCode.addCode(label2 + " :");

				praser_statement(statement);

				innerCode.addCode("GOTO " + label1);
				innerCode.addCode(label3 + " :");

				//弹出添加的block
				blockStack.pop_back();
			}
			//FOR ( expression_statement expression_statement expression ) statement
			else if (node->left->right->right->right->right->name == "expression") {
				//添加一个新的block
				Block newblock;
				blockStack.push_back(newblock);

				gramTree* exp_state1 = node->left->right->right;
				gramTree* exp_state2 = exp_state1->right;
				gramTree* exp = exp_state2->right;
				gramTree* statement = exp->right->right;

				string label1 = innerCode.getLabelName();
				string label2 = innerCode.getLabelName();
				string label3 = innerCode.getLabelName();

				if (exp_state1->left->name == "expression") {
					praser_expression(exp_state1->left);
				}
				innerCode.addCode(label1 + " :");

				varNode var;
				if (exp_state2->left->name == "expression") {
					var = praser_expression(exp_state2->left);
					innerCode.addCode("IF " + innerCode.getNodeName(var) + " != " + "#0 GOTO " + label2);
				}
				else {
					innerCode.addCode("GOTO " + label2);
				}

				innerCode.addCode("GOTO " + label3);
				innerCode.addCode(label2 + " :");

				praser_statement(statement);

				praser_expression(exp);

				innerCode.addCode("GOTO " + label1);
				innerCode.addCode(label3 + " :");

				//弹出添加的block
				blockStack.pop_back();
			}
		}
		if (node->left->right->right->name == "declaration") {
			//FOR '(' declaration expression_statement ')' statement
			if (node->left->right->right->right->right->name == ")") {
				//添加一个新的block
				Block newblock;
				blockStack.push_back(newblock);

				gramTree *declaration = node->left->right->right;
				gramTree *expression_statement = declaration->right;
				gramTree *statement = expression_statement->right->right;

				string label1 = innerCode.getLabelName();
				string label2 = innerCode.getLabelName();
				string label3 = innerCode.getLabelName();

				praser_declaration(declaration);
				innerCode.addCode(label1 + " :");

				varNode var;
				if (expression_statement->left->name == "expression") {
					var = praser_expression(expression_statement->left);
					innerCode.addCode("IF " + innerCode.getNodeName(var) + " != " + "#0 GOTO " + label2);
				}
				else {
					innerCode.addCode("GOTO " + label2);
				}
				innerCode.addCode("GOTO " + label3);
				innerCode.addCode(label2 + " :");

				praser_statement(statement);

				//cout << "here" << endl;
				innerCode.addCode("GOTO " + label1);
				innerCode.addCode(label3 + " :");

				//弹出添加的block
				blockStack.pop_back();

			}
			//FOR ( declaration expression_statement expression ) statement
			else if (node->left->right->right->right->right->name == "expression") {
				//添加一个新的block
				Block newblock;
				blockStack.push_back(newblock);

				gramTree *declaration = node->left->right->right;
				gramTree *expression_statement = declaration->right;
				gramTree *expression = expression_statement->right;
				gramTree *statement = expression->right->right;

				string label1 = innerCode.getLabelName();
				string label2 = innerCode.getLabelName();
				string label3 = innerCode.getLabelName();

				praser_declaration(declaration);
				innerCode.addCode(label1 + " :");

				varNode var;
				if (expression_statement->left->name == "expression") {
					var = praser_expression(expression_statement->left);
					innerCode.addCode("IF " + innerCode.getNodeName(var) + " != " + "#0 GOTO " + label2);
				}
				else {
					innerCode.addCode("GOTO " + label2);
				}
				innerCode.addCode("GOTO " + label3);
				innerCode.addCode(label2 + " :");

				praser_statement(statement);

				praser_expression(expression);
				//cout << "here" << endl;
				innerCode.addCode("GOTO " + label1);
				innerCode.addCode(label3 + " :");


				//弹出添加的block
				blockStack.pop_back();
			}
		}
	}
}

//函数定义
struct gramTree* Praser::praser_function_definition(struct gramTree* node) {
	gramTree* type_specifier = node->left;
	gramTree* declarator = node->left->right;
	gramTree* compound_statement = declarator->right;
	
	string funcType = type_specifier->left->content;
	string funcName = declarator->left->left->content;

	if (build_in_function.find(funcName) != build_in_function.end()) {
		error(declarator->left->left->line, "Function name can't be bulid in function.");
	}

	bool isdeclared = false;
	funcNode declarFunc;
	if (funcPool.find(funcName) != funcPool.end()) {
		//函数重复定义
		if (funcPool[funcName].isdefinied) {
			error(declarator->left->left->line, "Function " + funcName + " is duplicated definition.");
		}
		//函数事先声明过但是没有定义
		else {
			isdeclared = true;
			//先删除掉函数池中的函数的声明
			declarFunc = funcPool[funcName];
			funcPool.erase(funcPool.find(funcName));
		}
	}

	//进入新的block
	Block funBlock;
	funBlock.isfunc = true;
	funBlock.func.name = funcName;
	funBlock.func.rtype = funcType;
	funBlock.func.isdefinied = true;
	//将函数记录在块内并添加到函数池
	blockStack.push_back(funBlock);
	funcPool.insert({funcName,funBlock.func});

	innerCode.addCode("FUNCTION " + funcName + " :");

	//获取函数形参列表
	if(declarator->left->right->right->name == "parameter_list")
		praser_parameter_list(declarator->left->right->right, funcName);

	//此时函数池中的func已经添加了参数列表
	funcNode func = funcPool[funcName];
	//如果函数事先声明过，则比较函数的参数列表和返回类型
	if (isdeclared) {
		if (func.rtype != declarFunc.rtype) {
			error(type_specifier->left->line, "Function return type doesn't equal to the function declared before.");
		}
		cout << funBlock.func.paralist.size() << endl;
		if (func.paralist.size() != declarFunc.paralist.size()) {
			error(declarator->left->right->right->line, "The number of function parameters doesn't equal to the function declared before.");
		}
		for (int i = 0; i < funBlock.func.paralist.size(); i++) {
			if (func.paralist[i].type != declarFunc.paralist[i].type)
				error(declarator->left->right->right->line, "The parameter " + funBlock.func.paralist[i].name + "'s type doesn't equal to the function declared before." );
		}
	}
	//更新Block中func的参数列表
	funBlock.func = func;
	//分析函数的正文
	praser_compound_statement(compound_statement);

	//函数结束后，弹出相应的block
	blockStack.pop_back();

	return node->right;
}

//获取函数形参列表
void Praser::praser_parameter_list(struct gramTree* node,string funcName) {
	if (node->left->name == "parameter_list") {
		praser_parameter_list(node->left, funcName);
	}
	else if (node->left->name == "parameter_declaration") {
		praser_parameter_declaration(node->left,funcName);
	}

	if (node->right->name == ",") {
		praser_parameter_declaration(node->right->right, funcName);
	}
}

//获取单个形参内容
void Praser::praser_parameter_declaration(struct gramTree* node, string funcName) {
	//cout << "praser_parameter_declaration" << endl;
	gramTree* type_specifier = node->left;
	gramTree* declarator = node->left->right;
	string typeName = type_specifier->left->content;
	if (typeName == "void") {
		error(type_specifier->line, "Void can't definite parameter.");
	}
	//================================================
	//暂时只考虑变量，不考虑数组作为形参
	string varName = declarator->left->content;
	varNode newnode;
	newnode.name = varName;
	newnode.type = typeName;
	newnode.num = innerCode.varNum++;
	blockStack.back().func.paralist.push_back(newnode);
	funcPool[funcName].paralist.push_back(newnode);
	
	//将函数的形参添加到当前块的变量池中
	blockStack.back().varMap.insert({varName,newnode});

	innerCode.addCode(innerCode.createCodeforParameter(newnode));
}


struct gramTree* Praser::praser_declaration(struct gramTree *node) {
	//cout << "at " << node->name << endl;
	//node = declaration
	struct gramTree* begin = node->left;	//begin:type_specifier
	if (begin->right->name == ";")
		return node->right;
	
	string vartype = begin->left->content;

	if (vartype == "void") {
		error(begin->left->line,"void type can't assign to variable");	//报错
 	}
	struct gramTree* decl = begin->right;	//init_declarator_list


	/*while (decl->right) {
		praser_init_declarator(vartype, decl->right->right);
		decl = decl->left;
	}
	praser_init_declarator(vartype, decl);*/
	praser_init_declarator_list(vartype, decl);
	return node->right;

}

void Praser::praser_init_declarator_list(string vartype, struct gramTree* node) {
	if (node->left->name == "init_declarator_list") {
		praser_init_declarator_list(vartype, node->left);
	}
	else if (node->left->name == "init_declarator") {
		praser_init_declarator(vartype, node->left);
	}

	if (node->right->name == ",") {
		praser_init_declarator(vartype, node->right->right);
	}
}


//分析变量初始化,不确定有无定义
void Praser::praser_init_declarator(string vartype, struct gramTree* node) {
	//cout << "at " << node->name << endl;
	struct gramTree* declarator = node->left;

	if (!declarator->right) {
		//获取变量的名字
		if (declarator->left->name == "IDENTIFIER") {
			struct gramTree* id = declarator->left;
			string var = id->content;
			if (!lookupCurruntVar(var)) {
				varNode newvar;
				newvar.name = var;
				newvar.type = vartype;
				newvar.num = innerCode.varNum++;
				blockStack.back().varMap.insert({ var,newvar });
			}
			else error(declarator->left->line, "Variable multiple declaration.");
		}
		else {
			//函数声明
			if (declarator->left->right->name == "(") {
				string funcName = declarator->left->left->content;
				string funcType = vartype;
				if (blockStack.size() > 1) {
					error(declarator->left->right->line, "Functinon declaration must at global environment.");
				}
				gramTree* parameter_list = declarator->left->right->right;
				funcNode newFunc;
				newFunc.isdefinied = false;
				newFunc.name = funcName;
				newFunc.rtype = funcType;
				funcPool.insert({ funcName,newFunc });
				//分析函数形参列表
				praser_parameter_list(parameter_list,funcName);
			}
		}
	}
	else if (declarator->right->name == "=") {	//有初始化
		//获取变量的名字
		varNode newvar;
		if (declarator->left->name == "IDENTIFIER") {
			struct gramTree* id = declarator->left;
			string var = id->content;
			if (!lookupCurruntVar(var)) {
				newvar.name = var;
				newvar.type = vartype;
				newvar.num = innerCode.varNum++;
				blockStack.back().varMap.insert({ var,newvar });
			}
			else error(declarator->left->line, "Variable multiple declaration.");
		}
		else error(declarator->left->line, "It's not a variable!");


		gramTree* initializer = declarator->right->right;
		if (initializer == NULL) {
			error(declarator->line, "Lack the initializer for variable.");
		}
		else {
			if (initializer->left->name == "assignment_expression") {
				varNode rnode = praser_assignment_expression(initializer->left);
				innerCode.addCode(innerCode.createCodeforAssign(newvar,rnode));
				string rtype = rnode.type;
				if (rtype != vartype)
					error(initializer->left->line, "Wrong type to variable " + declarator->left->content + ": " + 
					rtype + " to " + vartype);
			}
		}
	}
	else error(declarator->right->line, "Wrong value to variable");
}

varNode Praser::praser_assignment_expression(struct gramTree* assign_exp) {	//返回变量节点

	//cout << "praser_assignment_expression" << endl;

	if (assign_exp->left->name == "logical_or_expression") {
		struct gramTree* logical_or_exp = assign_exp->left;

		return praser_logical_or_expression(logical_or_exp);
	}
	//赋值运算
	else if(assign_exp->left->name == "unary_expression"){
		struct gramTree* unary_exp = assign_exp->left;
		string op = assign_exp->left->right->left->name;
		struct gramTree* next_assign_exp = assign_exp->left->right->right;
		varNode node1 = praser_unary_expression(unary_exp);
		varNode node2 = praser_assignment_expression(next_assign_exp);
		varNode node3;
		if (op == "=") {
			node3 = node2;
		}
		else {
			string tempname = "_temp" + inttostr(innerCode.tempNum);
			++innerCode.tempNum;
			node3 = createTempVar(tempname, node1.type);

			blockStack.back().varMap.insert({ tempname,node3 });

			if (op == "MUL_ASSIGN") { //*=
				if (node1.type != node2.type) {
					error(assign_exp->left->line, "Different type for two variables.");
				}
				innerCode.addCode(innerCode.createCodeforVar(tempname, "*", node1, node2));
			}
			else if (op == "DIV_ASSIGN") { //*=
				if (node1.type != node2.type) {
					error(assign_exp->left->line, "Different type for two variables.");
				}
				innerCode.addCode(innerCode.createCodeforVar(tempname, "/", node1, node2));
			}
			else if (op == "MOD_ASSIGN") { //*=
				if (node1.type != "int" || node2.type != "int") {
					error(assign_exp->left->line, "The two variables must be int.");
				}
				innerCode.addCode(innerCode.createCodeforVar(tempname, "%", node1, node2));
			}
			else if (op == "ADD_ASSIGN") { //*=
				if (node1.type != node2.type) {
					error(assign_exp->left->line, "Different type for two variables.");
				}
				innerCode.addCode(innerCode.createCodeforVar(tempname, "+", node1, node2));
			}
			else if (op == "SUB_ASSIGN") { //*=
				if (node1.type != node2.type) {
					error(assign_exp->left->line, "Different type for two variables.");
				}
				innerCode.addCode(innerCode.createCodeforVar(tempname, "-", node1, node2));
			}
			else if (op == "LEFT_ASSIGN") { //*=
				if (node1.type != "int" || node2.type != "int") {
					error(assign_exp->left->line, "The two variables must be int.");
				}
				innerCode.addCode(innerCode.createCodeforVar(tempname, "<<", node1, node2));
			}
			else if (op == "RIGHT_ASSIGN") { //*=
				if (node1.type != "int" || node2.type != "int") {
					error(assign_exp->left->line, "The two variables must be int.");
				}
				innerCode.addCode(innerCode.createCodeforVar(tempname, ">>", node1, node2));
			}
			else if (op == "AND_ASSIGN") { //*=
				if (node1.type != "int" || node2.type != "int") {
					error(assign_exp->left->line, "The two variables must be int.");
				}
				innerCode.addCode(innerCode.createCodeforVar(tempname, "&", node1, node2));
			}
			else if (op == "XOR_ASSIGN") { //*=
				if (node1.type != "int" || node2.type != "int") {
					error(assign_exp->left->line, "The two variables must be int.");
				}
				innerCode.addCode(innerCode.createCodeforVar(tempname, "^", node1, node2));
			}
			else if (op == "OR_ASSIGN") { //*=
				if (node1.type != "int" || node2.type != "int") {
					error(assign_exp->left->line, "The two variables must be int.");
				}
				innerCode.addCode(innerCode.createCodeforVar(tempname, "|", node1, node2));
			}
		}

		innerCode.addCode(innerCode.createCodeforAssign(node1, node3));
		return node1;
	}
}

varNode Praser::praser_logical_or_expression(struct gramTree* logical_or_exp) {

	if(logical_or_exp->left->name == "logical_and_expression"){
		struct gramTree* logical_and_exp = logical_or_exp->left;
		return praser_logical_and_expression(logical_and_exp);
	}
	else if (logical_or_exp->left->name == "logical_or_expression") {
		//logical_or_expression -> logical_or_expression OR_OP logical_and_expression
		varNode node1 = praser_logical_or_expression(logical_or_exp->left);
		varNode node2 = praser_logical_and_expression(logical_or_exp->left->right->right);

		if (node1.type != "bool" || node2.type != "bool") {
			error(logical_or_exp->left->right->line, "Logical Or operation should only used to bool. ");
		}

		string tempname = "_temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;
		varNode newnode = createTempVar(tempname, node1.type);

		blockStack.back().varMap.insert({ tempname,newnode });
		innerCode.addCode(innerCode.createCodeforVar(tempname, "||", node1, node2));
		return newnode;

	}

}

varNode Praser::praser_logical_and_expression(struct gramTree* logical_and_exp) {
	
	if (logical_and_exp->left->name == "inclusive_or_expression") {
		gramTree* inclusive_or_exp = logical_and_exp->left;
		return praser_inclusive_or_expression(inclusive_or_exp);
	}
	else if (logical_and_exp->left->name == "logical_and_expression") {
		varNode node1 = praser_logical_and_expression(logical_and_exp->left);
		varNode node2 = praser_inclusive_or_expression(logical_and_exp->left->right->right);

		if (node1.type != "bool" || node2.type != "bool") {
			error(logical_and_exp->left->right->line, "Logical And operation should only used to bool. ");
		}

		string tempname = "_temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;
		varNode newnode = createTempVar(tempname, node1.type);
		blockStack.back().varMap.insert({ tempname,newnode });
	
		innerCode.addCode(innerCode.createCodeforVar(tempname, "&&", node1, node2));
		return newnode;

	}
}

varNode Praser::praser_inclusive_or_expression(struct gramTree* inclusive_or_exp) {
	
	if (inclusive_or_exp->left->name == "exclusive_or_expression") {
		gramTree* exclusive_or_exp = inclusive_or_exp->left;
		return praser_exclusive_or_expression(exclusive_or_exp);
	}
	else if (inclusive_or_exp->left->name == "inclusive_or_expression") {
		varNode node1 = praser_inclusive_or_expression(inclusive_or_exp->left);
		varNode node2 = praser_exclusive_or_expression(inclusive_or_exp->left->right->right);

		if (node1.type != "int" || node2.type != "int") {
			error(inclusive_or_exp->left->right->line, "Inclusive Or operation should only used to int. ");
		}

		string tempname = "_temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;
		varNode newnode = createTempVar(tempname, node1.type);
		blockStack.back().varMap.insert({ tempname,newnode });
		innerCode.addCode(innerCode.createCodeforVar(tempname, "|", node1, node2));
		return newnode;
	}
}

varNode Praser::praser_exclusive_or_expression(struct gramTree *exclusive_or_exp) {
	
	if (exclusive_or_exp->left->name == "and_expression") {
		gramTree* and_exp = exclusive_or_exp->left;
		return praser_and_expression(and_exp);
	}
	else if (exclusive_or_exp->left->name == "exclusive_or_expression") {
		varNode node1 = praser_exclusive_or_expression(exclusive_or_exp->left);
		varNode node2 = praser_and_expression(exclusive_or_exp->left->right->right);

		if (node1.type != "int" || node2.type != "int") {
			error(exclusive_or_exp->left->right->line, "Exclusive Or operation should only used to int. ");
		}

		string tempname = "_temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;
		varNode newnode = createTempVar(tempname, node1.type);
		blockStack.back().varMap.insert({ tempname,newnode });
		innerCode.addCode(innerCode.createCodeforVar(tempname, "^", node1, node2));
		return newnode;
	}
}

varNode Praser::praser_and_expression(struct gramTree* and_exp) {
	if (and_exp->left->name == "equality_expression") {
		gramTree* equality_exp = and_exp->left;
		return praser_equality_expression(equality_exp);
	}
	else if (and_exp->left->name == "and_expression") {
		varNode node1 = praser_and_expression(and_exp->left);
		varNode node2 = praser_equality_expression(and_exp->left->right->right);

		if (node1.type != "int" || node2.type != "int") {
			error(and_exp->left->right->line, "And operation should only used to int. ");
		}

		string tempname = "_temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;

		varNode newnode = createTempVar(tempname, node1.type);

		blockStack.back().varMap.insert({ tempname,newnode });
		innerCode.addCode(innerCode.createCodeforVar(tempname, "&", node1, node2));
		return newnode;
	}
}

varNode Praser::praser_equality_expression(struct gramTree* equality_exp) {
	
	if (equality_exp->left->name == "relational_expression") {
		gramTree* relational_exp = equality_exp->left;
		return praser_relational_expression(relational_exp);
	}
	else if (equality_exp->left->right->name == "EQ_OP" || equality_exp->left->right->name == "NE_OP") {
		string op;
		if (equality_exp->left->right->name == "EQ_OP")
			op = "==";
		else op = "!=";

		varNode node1 = praser_equality_expression(equality_exp->left);
		varNode node2 = praser_relational_expression(equality_exp->left->right->right);

		if (node1.type != node2.type) {
			error(equality_exp->left->right->line, "Different type for two variables.");
		}

		string tempname = "_temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;

		varNode newnode = createTempVar(tempname, "bool");
		blockStack.back().varMap.insert({ tempname,newnode});
		innerCode.addCode(innerCode.createCodeforVar(tempname, op, node1, node2));
		return newnode;
	}
}

varNode Praser::praser_relational_expression(struct gramTree* relational_exp) {
	if (relational_exp->left->name == "shift_expression") {
		gramTree* shift_exp = relational_exp->left;
		return praser_shift_expression(shift_exp);
	}
	else {
		string op = relational_exp->left->right->name;
		if (op == "LE_OP")
			op = "<=";
		else if (op == "GE_OP")
			op = ">=";
		if (op == ">" || op == "<" || op == ">=" || op == "<=") {
			varNode node1 = praser_relational_expression(relational_exp->left);
			varNode node2 = praser_shift_expression(relational_exp->left->right->right);

			if (node1.type != node2.type) {
				error(relational_exp->left->right->line, "Different type for two variables.");
			}

			string tempname = "_temp" + inttostr(innerCode.tempNum);
			++innerCode.tempNum;

			varNode newnode = createTempVar(tempname, "bool");
			blockStack.back().varMap.insert({ tempname,newnode });
			innerCode.addCode(innerCode.createCodeforVar(tempname, op, node1, node2));
			return newnode;
		}
	}
}

varNode Praser::praser_shift_expression(struct gramTree*shift_exp) {
	if (shift_exp->left->name == "additive_expression") {
		gramTree* additive_exp = shift_exp->left;
		return praser_additive_expression(additive_exp);
	}
	else if (shift_exp->left->right->name == "LEFT_OP" || shift_exp->left->right->name == "RIGHT_OP") {
		string op;
		if (shift_exp->left->right->name == "LEFT_OP") {
			op = "<<";
		}
		else op = ">>";

		varNode node1 = praser_shift_expression(shift_exp->left);
		varNode node2 = praser_additive_expression(shift_exp->left->right->right);

		if (node1.type != "int" || node2.type != "int" ) {
			error(shift_exp->left->right->line, "Shift operation should only used to int. ");
		}

		string tempname = "_temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;

		varNode newnode = createTempVar(tempname, node1.type);

		blockStack.back().varMap.insert({ tempname,newnode });

		innerCode.addCode(innerCode.createCodeforVar(tempname, op, node1, node2));
		return newnode;
	}
}

varNode Praser::praser_additive_expression(struct gramTree* additive_exp) {
	if (additive_exp->left->name == "multiplicative_expression") {
		gramTree* mult_exp = additive_exp->left;
		return praser_multiplicative_expression(mult_exp);
	}
	else if (additive_exp->left->right->name == "+" || additive_exp->left->right->name == "-") {
		varNode node1 = praser_additive_expression(additive_exp->left);
		varNode node2 = praser_multiplicative_expression(additive_exp->left->right->right);

		if (node1.type != node2.type) {
			error(additive_exp->left->right->line, "Different type for two variables.");
		}

		string tempname = "_temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;
		varNode newnode = createTempVar(tempname, node1.type);
		blockStack.back().varMap.insert({ tempname,newnode});

		innerCode.addCode(innerCode.createCodeforVar(tempname, additive_exp->left->right->name, node1, node2));
		return newnode;
	}
}

varNode Praser::praser_multiplicative_expression(struct gramTree* mult_exp) {

	if (mult_exp->left->name == "unary_expression") {
		gramTree* unary_exp = mult_exp->left;
		return praser_unary_expression(unary_exp);
	}
	else if (mult_exp->left->right->name == "*" || mult_exp->left->right->name == "/" || 
		mult_exp->left->right->name == "%") {
		varNode node1 = praser_multiplicative_expression(mult_exp->left);
		varNode node2 = praser_unary_expression(mult_exp->left->right->right);

		if (node1.type != node2.type) {
			error(mult_exp->left->right->line, "Different type for two variables.");
		}

		string tempname = "_temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;
		varNode newNode = createTempVar(tempname, node1.type);
		blockStack.back().varMap.insert({ tempname,newNode });

		innerCode.addCode(innerCode.createCodeforVar(tempname, mult_exp->left->right->name,node1,node2));
		return newNode;

	}
}

varNode Praser::praser_unary_expression(struct gramTree*unary_exp) {
	if (unary_exp->left->name == "postfix_expression") {
		gramTree* post_exp = unary_exp->left;
		return praser_postfix_expression(post_exp);
	}
	else if (unary_exp->left->name == "INC_OP") {

	}
	else if (unary_exp->left->name == "DEC_OP") {

	}
	else if (unary_exp->left->name == "unary_operator") {

	}
}

varNode Praser::praser_postfix_expression(struct gramTree* post_exp) {
	if (post_exp->left->name == "primary_expression") {
		gramTree* primary_exp = post_exp->left;
		return praser_primary_expression(primary_exp);
	}
	else if (post_exp->left->right->name == "[") {
		//数组调用
	}
	else if (post_exp->left->right->name == "(") {
		//函数调用
		string funcName = post_exp->left->left->left->content;
		varNode newNode;
		if (build_in_function.find(funcName) != build_in_function.end()) {
			if (funcName == "read") {
				string tempname = "_temp" + inttostr(innerCode.tempNum);
				++innerCode.tempNum;

				newNode = createTempVar(tempname, "int");
				innerCode.addCode("READ " + tempname);
			}
			else if (funcName == "write") {
				gramTree* argu_exp_list = post_exp->left->right->right;
				if (argu_exp_list->left->name != "assignment_expression") {
					error(argu_exp_list->left->line, "function write has just one parameter");
				}
				varNode rnode = praser_assignment_expression(argu_exp_list->left);
				innerCode.addCode("WRITE " + rnode.name);
				return rnode;
			}
		}
		else {
			if (funcPool.find(funcName) == funcPool.end()) {
				error(post_exp->left->left->left->line, "Undefined function " + funcName);
			}

			if (post_exp->left->right->right->name == "argument_expression_list") {
				gramTree* argument_exp_list = post_exp->left->right->right;
				praser_argument_expression_list(argument_exp_list, funcName);
				//cout << "funcCall" << endl;

			}
			string tempname = "_temp" + inttostr(innerCode.tempNum);
			++innerCode.tempNum;

			newNode = createTempVar(tempname, funcPool[funcName].rtype);
			innerCode.addCode(tempname + " := CALL " + funcName);
		}

		return newNode;
		
	}
	else if (post_exp->left->right->name == "INC_OP") {

	}
	else if (post_exp->left->right->name == "DEC_OP") {

	}
}

void Praser::praser_argument_expression_list(struct gramTree* node, string funcName) {
	gramTree* argu_exp_list = node->left;
	funcNode func = funcPool[funcName];
	int i = 0;
	while (argu_exp_list->name == "argument_expression_list") {
		varNode rnode = praser_assignment_expression(argu_exp_list->right->right);

		innerCode.addCode(innerCode.createCodeforArgument(rnode));

		argu_exp_list = argu_exp_list->left;
		i++;
		if (func.paralist[func.paralist.size() - i].type != rnode.type) {
			error(argu_exp_list->line, "Wrong type arguments to function " + funcName);
		}
	}
	varNode rnode = praser_assignment_expression(argu_exp_list);
	innerCode.addCode(innerCode.createCodeforArgument(rnode));
	i++;
	if (func.paralist[func.paralist.size() - i].type != rnode.type) {
		error(argu_exp_list->line, "Wrong type arguments to function " + funcName);
	}
	if (i != func.paralist.size()) {
		error(argu_exp_list->line, "The number of arguments doesn't equal to the function parameters number.");
	}
}

varNode Praser::praser_primary_expression(struct gramTree* primary_exp) {
	if (primary_exp->left->name == "IDENTIFIER") {
		string content = primary_exp->left->content;
		varNode rnode = lookupNode(content);
		if (rnode.num < 0) {
			error(primary_exp->left->line, "Undefined variable " + content);
		}
		return rnode;
	}
	else if (primary_exp->left->name == "TRUE" || primary_exp->left->name == "FALSE") {
		string content = primary_exp->left->content;
		string tempname = "_temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;
		varNode newNode = createTempVar(tempname, "bool");
		blockStack.back().varMap.insert({ tempname,newNode });
		if(primary_exp->left->name == "TRUE") 
			innerCode.addCode(tempname + " := #1");
		else {
			innerCode.addCode(tempname + " := #0");
		}
		return newNode;
	}
	else if (primary_exp->left->name == "CONSTANT_INT") {
		string content = primary_exp->left->content;
		string tempname = "_temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;
		
		varNode newNode = createTempVar(tempname, "int");
		blockStack.back().varMap.insert({ tempname,newNode });
		innerCode.addCode(tempname + " := #"  + content);
		return newNode;
	}
	else if (primary_exp->left->name == "CONSTANT_DOUBLE") {
		string content = primary_exp->left->content;
		string tempname = "_temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;

		varNode newNode = createTempVar(tempname, "double");

		blockStack.back().varMap.insert({ tempname,newNode});
		innerCode.addCode(tempname + " := F" + content);
		return newNode;
	}
	else if (primary_exp->left->name == "(") {
		struct gramTree* expression = primary_exp->left->right;
		return praser_expression(expression);
	}
}


//全局查找
string Praser::lookupVar(string name) {
	int N = blockStack.size();
	for (int i = N - 1; i >= 0; i--) {
		if (blockStack[i].varMap.find(name) != blockStack[i].varMap.end())
			return blockStack[i].varMap[name].type;
	}
	return "";
}
//当前块查找
bool Praser::lookupCurruntVar(string name) {
	return blockStack.back().varMap.find(name) != blockStack.back().varMap.end();
}

struct varNode Praser::lookupNode(string name) {
	int N = blockStack.size();
	for (int i = N - 1; i >= 0; i--) {
		if (blockStack[i].varMap.find(name) != blockStack[i].varMap.end())
			return blockStack[i].varMap[name];
	}
	varNode temp;
	temp.num = -1;
	return temp;
}

string Praser::getFuncRType() {
	int N = blockStack.size();
	for (int i = N - 1; i >= 0; i--) {
		if (blockStack[i].isfunc)
			return blockStack[i].func.rtype;
	}
	return "";
}

void Praser::error(int line, string error) {

	print_code();

	cout << "Error! line " << line << ": ";
	cout << error << endl;
	exit(1);
}

struct varNode Praser::createTempVar(string name, string type) {
	varNode var;
	var.name = name;
	var.type = type;
	var.num = -1;
	return var;
}

void Praser::print_map() {
	int N = blockStack.size();
	for (int i = N - 1; i >= 0; i--) {
		cout << "Block " << i << endl;
		for (auto it = blockStack[i].varMap.begin(); it != blockStack[i].varMap.end(); it++) {
			cout << "     " << it->first << " " << it->second.type << endl;
		}
	}
}

void Praser::print_code() {
	innerCode.printCode();
}

