#include "checker.h"
#include "intrinsic.h"

// TODO: Currently, I assume that the checker will never actually modify
//       the APM, only read it. It may be worth formalising that assumption
//       by making the parameters to each method immutable.

// TODO: As an item of curiosity, I'm sure that checking actually has to be
//       done as a tree walk? If the checker never modifies the APM, than the
//       order in which nodes are checked should not effect the outcome.
//
//       In theory then, homogenous nodes could be put in arrays and iterated
//       over to be checked, instead of doing a tree walk. It's not immediately
//       clear to me however if this would be more performant? On one hand, this
//       approach deploy memory more efficiently. On the other, using tree decent
//       may make it easier to avoid duplicate work? (I'm also not sure how
//       homogenous the nodes would really be?)

void Checker::check(Source &source, ptr<Program> program)
{
    this->source = &source;
    check_program(program);
}

// PROGRAM STRUCTURE //

void Checker::check_program(ptr<Program> program)
{
    check_scope(program->global_scope);
}

void Checker::check_scope(ptr<Scope> scope)
{
    for (auto index : scope->lookup)
        check_scope_lookup_value(index.second, scope);
}

void Checker::check_scope_lookup_value(Scope::LookupValue value, ptr<Scope> scope)
{

    if (IS_PTR(value, Variable))
    {
        // pass
    }

    else if (IS_PTR(value, UnionPattern))
    {
        // pass
    }

    else if (IS_PTR(value, IntrinsicType))
    {
        // pass
    }

    else if (IS_PTR(value, EnumType))
    {
        // pass
    }

    else if (IS_PTR(value, Entity))
    {
        // pass
    }

    else if (IS_PTR(value, StateProperty))
    {
        auto state = AS_PTR(value, StateProperty);
        if (state->initial_value.has_value())
        {
            auto initial_value = state->initial_value.value();
            check_expression(initial_value, state->scope);

            auto initial_value_pattern = determine_expression_pattern(initial_value);
            if (!is_pattern_subset_of_superset(initial_value_pattern, state->pattern))
                source->log_error("Default value for state is the incorrect type.", get_span(initial_value));
        }
    }

    else if (IS_PTR(value, FunctionProperty))
    {
        auto funct = AS_PTR(value, FunctionProperty);
        if (funct->body.has_value())
            check_code_block(funct->body.value());

        // FIXME: If the body is a singleton, check the statement as if it were a return expression
    }

    else if (IS_PTR(value, Procedure))
    {
        auto proc = AS_PTR(value, Procedure);
        check_code_block(proc->body);

        // FIXME: If the body is a singleton, check the statement as if it were a return expression
    }

    else if (IS_PTR(value, Scope::OverloadedIdentity))
    {
        auto overloaded_identity = AS_PTR(value, Scope::OverloadedIdentity);
        for (auto overload : overloaded_identity->overloads)
            check_scope_lookup_value(overload, scope);

        // TODO: Check that no overloads share the same signature
    }

    else
    {
        throw CompilerError("Unable to check Scope Lookup Value");
    }
}

void Checker::check_code_block(ptr<CodeBlock> code_block)
{
    check_scope(code_block->scope);
    for (auto stmt : code_block->statements)
        check_statement(stmt, code_block->scope);
}

// STATEMENTS //

void Checker::check_statement(Statement stmt, ptr<Scope> scope)
{
    if (IS(stmt, Expression))
        check_expression(AS(stmt, Expression), scope);
    else if (IS_PTR(stmt, CodeBlock))
        check_code_block(AS_PTR(stmt, CodeBlock));
    else if (IS_PTR(stmt, IfStatement))
        check_if_statement(AS_PTR(stmt, IfStatement), scope);
    else if (IS_PTR(stmt, ForStatement))
        check_for_statement(AS_PTR(stmt, ForStatement), scope);
    else if (IS_PTR(stmt, AssignmentStatement))
        check_assignment_statement(AS_PTR(stmt, AssignmentStatement), scope);
    else if (IS_PTR(stmt, VariableDeclaration))
        check_variable_declaration(AS_PTR(stmt, VariableDeclaration), scope);
    else
        throw CompilerError("Cannot check Statement variant.", get_span(stmt));
}

void Checker::check_if_statement(ptr<IfStatement> stmt, ptr<Scope> scope)
{
    for (auto rule : stmt->rules)
    {
        check_expression(rule.condition, scope);
        check_code_block(rule.code_block);

        auto condition_pattern = determine_expression_pattern(rule.condition);
        bool is_bool_condition = is_pattern_subset_of_superset(condition_pattern, Intrinsic::type_bool);
        bool is_optional_condition = is_pattern_optional(condition_pattern);
        if (!is_bool_condition && !is_optional_condition)
            source->log_error("Condition must evaluate either to true or false, or potentially to none. This condition will never be true, false, or none.", rule.span);
    }

    if (stmt->else_block.has_value())
        check_code_block(stmt->else_block.value());
}

void Checker::check_for_statement(ptr<ForStatement> stmt, ptr<Scope> scope)
{
    // TODO: Check that range is actually iterable
    check_code_block(stmt->body);
}

void Checker::check_assignment_statement(ptr<AssignmentStatement> stmt, ptr<Scope> scope)
{
    auto subject_pattern = determine_expression_pattern(stmt->subject);
    auto value_pattern = determine_expression_pattern(stmt->value);
    if (!is_pattern_subset_of_superset(value_pattern, subject_pattern))
        source->log_error("Assigned value does not match the pattern of the subject.", stmt->span);
}

void Checker::check_variable_declaration(ptr<VariableDeclaration> stmt, ptr<Scope> scope)
{
    if (stmt->value.has_value())
    {
        auto value_pattern = determine_expression_pattern(stmt->value.value());
        if (!is_pattern_subset_of_superset(value_pattern, stmt->variable->pattern))
            source->log_error("Assigned value does not match the pattern of the variable.", stmt->span);
    }
}

// EXPRESSIONS //

void Checker::check_expression(Expression expression, ptr<Scope> scope)
{

    if (IS_PTR(expression, UnresolvedIdentity))
        throw CompilerError("Attempt to check UnresolvedIdentity. This should have already been resolved.");
    else if (IS_PTR(expression, Variable))
        ; // skip
    else if (IS_PTR(expression, EnumValue))
        ; // skip
    else if (IS_PTR(expression, IntrinsicValue))
        ; // skip
    else if (IS_PTR(expression, ListValue))
        check_list_value(AS_PTR(expression, ListValue), scope);
    else if (IS_PTR(expression, InstanceList))
        check_instance_list(AS_PTR(expression, InstanceList), scope);
    else if (IS_PTR(expression, Unary))
        check_unary(AS_PTR(expression, Unary), scope);
    else if (IS_PTR(expression, Binary))
        check_binary(AS_PTR(expression, Binary), scope);
    else if (IS_PTR(expression, ExpressionIndex))
        check_expression_index(AS_PTR(expression, ExpressionIndex), scope);
    else if (IS_PTR(expression, PropertyIndex))
        check_property_index(AS_PTR(expression, PropertyIndex), scope);
    else if (IS_PTR(expression, Call))
        check_call(AS_PTR(expression, Call), scope);
    else if (IS_PTR(expression, IfExpression))
        check_if_expression(AS_PTR(expression, IfExpression), scope);
    else if (IS_PTR(expression, Match))
        check_match(AS_PTR(expression, Match), scope);
    else if (IS_PTR(expression, InvalidValue))
        ; // skip
    else if (IS_PTR(expression, InvalidExpression))
        ; // skip
    else
        throw CompilerError("Cannot check Expression variant.", get_span(expression));
}

void Checker::check_list_value(ptr<ListValue> list, ptr<Scope> scope)
{
    for (auto value : list->values)
        check_expression(value, scope);
}

void Checker::check_instance_list(ptr<InstanceList> list, ptr<Scope> scope)
{
    for (auto value : list->values)
        check_expression(value, scope);
}

void Checker::check_call(ptr<Call> call, ptr<Scope> scope)
{
    // TODO: Check callee / arguments

    for (auto &argument : call->arguments)
        check_expression(argument.value, scope);
}

void Checker::check_if_expression(ptr<IfExpression> if_expression, ptr<Scope> scope)
{
    for (auto &rule : if_expression->rules)
    {
        check_expression(rule.condition, scope);
        check_expression(rule.result, scope);

        auto condition_pattern = determine_expression_pattern(rule.condition);
        bool is_bool_condition = is_pattern_subset_of_superset(condition_pattern, Intrinsic::type_bool);
        bool is_optional_condition = is_pattern_optional(condition_pattern);
        if (!is_bool_condition && !is_optional_condition)
            source->log_error("Condition must evaluate either to true or false, or potentially to none. This condition will never be true, false, or none.", rule.span);
    }
}

void Checker::check_match(ptr<Match> match, ptr<Scope> scope)
{
    check_expression(match->subject, scope);
    auto subject_pattern = determine_expression_pattern(match->subject);

    for (auto &rule : match->rules)
    {
        // FIXME: Check that rule's pattern is static
        check_expression(rule.result, scope);

        if (!do_patterns_overlap(rule.pattern, subject_pattern))
            source->log_error("This rule's pattern will never match.", get_span(rule.pattern));
    }
}

void Checker::check_expression_index(ptr<ExpressionIndex> expression_index, ptr<Scope> scope)
{
    check_expression(expression_index->subject, scope);
    check_expression(expression_index->index, scope);

    // TODO: Check that the subject is an array/list
    // TODO: Check that the index is an integer
}

void Checker::check_property_index(ptr<PropertyIndex> property_index, ptr<Scope> scope)
{
    check_expression(property_index->expr, scope);
}

void Checker::check_unary(ptr<Unary> unary, ptr<Scope> scope)
{
    check_expression(unary->value, scope);
    // TODO: Pattern check the operand
}

void Checker::check_binary(ptr<Binary> binary, ptr<Scope> scope)
{
    check_expression(binary->lhs, scope);
    check_expression(binary->rhs, scope);
    // TODO: Pattern check the operands
}
