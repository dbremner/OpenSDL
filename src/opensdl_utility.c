/*
 * Copyright (C) Jonathan D. Belanger 2018.
 *
 *  OpenSDL is free software: you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free
 *  Software Foundation, either version 3 of the License, or (at your option)
 *  any later version.
 *
 *  OpenSDL is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with OpenSDL.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Description:
 *
 *  This source file contains the utility routines called during the parsing of
 *  the input file.
 *
 * Revision History:
 *
 *  V01.000	04-OCT-2018	Jonathan D. Belanger
 *  Initially written.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "opensdl_defs.h"
#include "opensdl_utility.h"
#include "opensdl_actions.h"

extern bool trace;

/*
 * sdl_get_local
 *  This function is called to get the value associated with a local definition
 *  currently defined with the indicated name.
 *
 * Input Parameters:
 *  context:
 *	A pointer to the context structure where we maintain information about
 *	the local variables.
 *  name:
 *	A pointer to the name of the local variable.
 *
 * Output Parameters:
 *  value:
 *	A pointer to the location to receive the value of the local variable.
 *
 * Return Value
 *  1:	Normal Successful Completion.
 *  0:	An error occurred.
 */
int sdl_get_local(SDL_CONTEXT *context, char *name, __int64_t *value)
{
    SDL_LOCAL_VARIABLE	*local = sdl_find_local(context, name);
    int			retVal = 1;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_get_local: ", __FILE__, __LINE__);

    /*
     * We should have already found the local variable definition
     */
    if ((local != NULL) && (value != NULL))
	*value = local->value;
    else
    {
	if (value != NULL)
	    value = 0;
	retVal = 0;
    }

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
    {
	if (retVal == 1)
	    printf(
		"%s, %ld (%016lx - %4.4s)",
		name,
		*value,
		*value,
		(char *) value);
	else
	    printf("%s not found", name);
	printf("\n");
    }

    /*
     * Return the results of this call back to the caller.
     */
    free(name);
    return(retVal);
}

/*
 * sdl_find_local
 *  This function is called to find the local definition currently defined with
 *  the indicated name.  If none if found a NULL is returned.
 *
 * Input Parameters:
 *  context:
 *	A pointer to the context structure where we maintain information about
 *	the local variables.
 *  name:
 *	A pointer to the name of the local variable.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value
 *  NULL:	Local variable not found.
 *  !NULL:	An existing local variable.
 */
SDL_LOCAL_VARIABLE *sdl_find_local(SDL_CONTEXT *context, char *name)
{
    SDL_LOCAL_VARIABLE	*retVal = (SDL_LOCAL_VARIABLE *) context->locals.flink;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_find_local\n", __FILE__, __LINE__);

    /*
     * Search through the list of local variables and see if one with the same
     * name already exists.
     */
    while (retVal != (SDL_LOCAL_VARIABLE *) &context->locals)
	if (strcmp(retVal->id, name) == 0)
	    break;
	else
	    retVal = (SDL_LOCAL_VARIABLE *) retVal->header.flink;

    /*
     * If we ended up at the head of the queue, then we did not find what we
     * were asked to locate.
     */
    if (retVal == (SDL_LOCAL_VARIABLE *) &context->locals)
	retVal = NULL;

    /*
     * Return the results of this call back to the caller.
     */
    return(retVal);
}

/*
 * sdl_state_transition
 *  This function is called with an action that allows for the potential
 *  transition of the parsing state machine.
 *
 * Input Parameters:
 *  context:
 *	A pointer to the context structure where we maintain information about
 *	the local variables.
 *  action:
 *	A value representing what is being parsed.  This may or may not cause a
 *	state transition.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value
 *  1:	Normal Successful Completion.
 *  0:	Action invalid in current state.
 */
int sdl_state_transition(SDL_CONTEXT *context, SDL_STATE action)
{
    int		retVal = 1;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf(
	    "%s:%d:sdl_state_transition(state: %d, action: %d)\n",
	    __FILE__,
	    __LINE__,
	    context->state,
	    action);

    /*
     * Anything that will not cause a state transition is not included below.
     */
    switch (context->state)
    {
	case Initial:

	    /*
	     * The only thing allowed in the initial state is the MODULE
	     * statement.	     */
	    switch (action)
	    {
		case Module:
		    context->state = Module;
		    break;

		default:
		    retVal = 0;
		    break;
	    }
	    break;

	case Module:

	    /*
	     * Most things are defined within a MODULE, so there will be many
	     * potential state transitions.
	     */
	    switch (action)
	    {
		case Declare:
		    context->state = Declare;
		    break;

		case Constant:
		    context->state = Constant;
		    context->constantPrevState = Module;
		    break;

		case Item:
		    context->state = Item;
		    break;

		case Aggregate:
		    context->state = Aggregate;
		    break;

		case Entry:
		    context->state = Entry;
		    break;

		case Local:
		    context->state = Local;
		    break;

		case DefinitionEnd:
		    context->state = Initial;
		    break;

		default:
		    retVal = 0;
		    break;
	    }
	    break;

	case Comment:
	    retVal = 0;
	    break;

	case Literal:
	    retVal = 0;
	    break;

	case Local:
	    if (action == DefinitionEnd)
		context->state = Module;
	    else
		retVal = 0;
	    break;

	case Declare:
	    if (action == DefinitionEnd)
	    {
		context->state = Module;
		sdl_declare_compl(context);
	    }
	    else
		retVal = 0;
	    break;

	case Constant:
	    switch (action)
	    {
		case DefinitionEnd:
		    context->state = context->constantPrevState;
		    sdl_constant_compl(context);
		    break;

		default:
		    retVal = 0;
		    break;
	    }
	    break;

	case Item:
	    if (action == DefinitionEnd)
	    {
		context->state = Module;
		sdl_item_compl(context);
	    }
	    else
		retVal = 0;
	    break;

	case Aggregate:
	    switch (action)
	    {
		case DefinitionEnd:
		    context->state = Module;
		    break;

		case Subaggregate:
		    context->state = Subaggregate;
		    break;

		case Constant:
		    context->state = Constant;
		    context->constantPrevState = Aggregate;
		    break;

		default:
		    retVal = 0;
		    break;
	    }
	    break;

	case Subaggregate:
	    switch (action)
	    {
		case DefinitionEnd:
		    switch (context->aggregateDepth)
		    {
			case 1:
			    context->state = Module;
			    break;

			case 2:
			    context->state = Aggregate;
			    break;

			default:
			    break;
		    }
		    break;

		case Subaggregate:
		    break;

		case Constant:
		    context->state = Constant;
		    context->constantPrevState = Subaggregate;
		    break;

		default:
		    retVal = 0;
		    break;
	    }
	    break;

	case Entry:
	    switch (action)
	    {
		case DefinitionEnd:
		    context->state = Module;
		    break;

		case Constant:
		    context->state = Constant;
		    context->constantPrevState = Entry;
		    break;

		default:
		    retVal = 0;
		    break;
	    }
	    break;

	case IfLanguage:
	    retVal = 0;
	    break;

	case IfSymbol:
	    retVal = 0;
	    break;

	case DefinitionEnd:
	    retVal = 0;
	    break;
    }

    /*
     * Return the results of this call back to the caller.
     */
    return(retVal);
}

/*
 * sdl_unquote_str
 *  This function is called to remove the leading and trailing double-quotes
 *  from the supplied string.  The call does the update in place.
 *
 * Input Parameters:
 *  str:
 *	A pointer to a string containing leading and trailing double quotes.
 *
 * Output Parameters:
 *  str:
 *	A pointer to a string no longer containing leading and trailing double
 *	quotes.
 *
 * Return Values:
 *  1:	Normal Successful Completion.
 *  0:	An error occurred.
 */
char *sdl_unquote_str(char *str)
{
    char	*retVal = str;
    int		ii;
    size_t	len = strlen(str);

    if (str[0] == '"')
    {
	for (ii = 0; ii < len; ii++)
	    str[ii] = str[ii+1];
	len--;
    }
    if (str[len - 1] == '"')
	str[len - 1] = '\0';

    /*
     * Return the results of this call back to the caller.
     */
    return(retVal);
}

/*
 * sdl_get_declare
 *  This function is called to get the record of a previously defined local
 *  variable.
 *
 * Input Parameters:
 *  declare:
 *	A pointer to the list containing all currently defined DECLAREs.
 *  typeID:
 *	A value assigned to the declared user-type.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value
 *  NULL:	Local variable not found.
 *  !NULL:	An existing local variable.
 */
SDL_DECLARE *sdl_get_declare(SDL_DECLARE_LIST *declare, int typeID)
{
    SDL_DECLARE	*retVal = (SDL_DECLARE *) declare->header.flink;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_get_declare\n", __FILE__, __LINE__);

    while (retVal != (SDL_DECLARE *) &declare->header)
	if (retVal->typeID == typeID)
	    break;
	else
	    retVal = (SDL_DECLARE *) retVal->header.flink;
    if (retVal == (SDL_DECLARE *) &declare->header)
	retVal = NULL;

    /*
     * Return the results of this call back to the caller.
     */
    return(retVal);
}

/*
 * sdl_get_item
 *  This function is called to get the record of a previously defined ITEM.
 *
 * Input Parameters:
 *  item:
 *	A pointer to the list containing all currently defined ITEMs.
 *  typeID:
 *	A value assigned to the declared item.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value
 *  NULL:	Local variable not found.
 *  !NULL:	An existing ITEM.
 */
SDL_ITEM *sdl_get_item(SDL_ITEM_LIST *item, int typeID)
{
    SDL_ITEM	*retVal = (SDL_ITEM *) item->header.flink;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_get_item\n", __FILE__, __LINE__);

    while (retVal != (SDL_ITEM *) &item->header)
	if (retVal->typeID == typeID)
	    break;
	else
	    retVal = (SDL_ITEM *) retVal->header.flink;
    if (retVal == (SDL_ITEM *) &item->header)
	retVal = NULL;

    /*
     * Return the results of this call back to the caller.
     */
    return(retVal);
}

/*
 * sdl_get_aggregate
 *  This function is called to get the record of a previously defined
 *  AGGREGATE.
 *
 * Input Parameters:
 *  aggregate:
 *	A pointer to the list containing all currently defined AGGREGATEs.
 *  typeID:
 *	A value assigned to the declared aggregate.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value
 *  NULL:	Local variable not found.
 *  !NULL:	An existing local variable.
 */
SDL_AGGREGATE *sdl_get_aggregate(SDL_AGGREGATE_LIST *aggregate, int typeID)
{
    SDL_AGGREGATE *retVal = (SDL_AGGREGATE *) aggregate->header.flink;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_get_aggregate\n", __FILE__, __LINE__);

    while (retVal != (SDL_AGGREGATE *) &aggregate->header)
	if (retVal->typeID == typeID)
	    break;
	else
	    retVal = (SDL_AGGREGATE *) retVal->header.flink;
    if (retVal == (SDL_AGGREGATE *) &aggregate->header)
	retVal = NULL;

    /*
     * Return the results of this call back to the caller.
     */
    return(retVal);
}

/*
 * sdl_usertype_idx
 *  This function is called to return the user type id associated with a
 *  particular user type.
 *
 * Input Parameters:
 *  context:
 *	A pointer to the context structure where we maintain information about
 *	the current parsing.
 *  usertype:
 *	A pointer to a string containing the name of the type.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value:
 *  A value greater than or equal to SDL_K_USER_MIN, representing the type ID
 *  of the indicated usertype.
 */
int sdl_usertype_idx(SDL_CONTEXT *context, char *usertype)
{
    int		retVal = 0;
    SDL_DECLARE *myDeclare = (SDL_DECLARE *) context->declares.header.flink;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_usertype_idx\n", __FILE__, __LINE__);

    while (myDeclare != (SDL_DECLARE *) &context->declares.header)
	if (strcmp(myDeclare->id, usertype) == 0)
	{
	    retVal = myDeclare->typeID;
	    break;
	}
	else
	    myDeclare = (SDL_DECLARE *) myDeclare->header.flink;

    /*
     * Return the results of this call back to the caller.
     */
    free(usertype);
    return(retVal);
}

/*
 * sdl_aggrtype_idx
 *  This function is called to return the aggregate type id associated with a
 *  particular aggregate name.
 *
 * Input Parameters:
 *  context:
 *	A pointer to the context structure where we maintain information about
 *	the current parsing.
 *  aggregateName:
 *	A pointer to a string containing the name of the AGGREGATE.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value:
 *  A value greater than or equal to SDL_K_USER_MIN, representing the type ID
 *  of the indicated aggregate type.
 */
int sdl_aggrtype_idx(SDL_CONTEXT *context, char *aggregateName)
{
    int			retVal = 0;
    SDL_AGGREGATE	*myAggregate =
			    (SDL_AGGREGATE *) context->aggregates.header.flink;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_aggrtype_idx\n", __FILE__, __LINE__);

    while (myAggregate != (SDL_AGGREGATE *) &context->aggregates.header)
	if (strcmp(myAggregate->id, aggregateName) == 0)
	{
	    retVal = myAggregate->typeID;
	    break;
	}
	else
	    myAggregate = (SDL_AGGREGATE *) myAggregate->header.flink;

    /*
     * Return the results of this call back to the caller.
     */
    free(aggregateName);
    return(retVal);
}

/*
 * sdl_bin2int
 *  This function is called to convert a binary number in to its equivalent
 *  integer value.
 *
 * Input Parameters:
 *  binVal:
 *  	A numeric value containing just 1's and 0's.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  A number representing the integer value converted from binary.
 */
__int64_t sdl_bin2int(char *binStr)
{
    __int64_t	retVal = 0;
    int		ii;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_bin2int\n", __FILE__, __LINE__);

    while (binStr[ii] != '\0')
    {
	retVal *= 2;
	retVal += binStr[ii++] - '0';
    }

    /*
     * Return the results of this call back to the caller.
     */
    free(binStr);
    return(retVal);
}

/*
 * sdl_str2int
 *  This function is called to convert a string to a value.  The string can be
 *  at most 8 characters long and the ASCII values are combined into a numeric
 *  value.  There is no attempt to interpret the contents of the string itself.
 *
 * Input Parameters:
 *  strVal:
 *	A pointer to the string to be converted.
 *
 * Output Parameters:
 *  val:
 *  	A pointer to a long integer location to receive the value of the
 *	converted string.
 *
 * Return Values:
 *  1:	Normal Successful Completion.
 *  0:	An error occurred.
 */
int sdl_str2int(char *strVal, __int64_t *val)
{
    int		retVal = 1;
    size_t	len = strlen(strVal);

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_str2int\n", __FILE__, __LINE__);

    /*
     * If the input string is less than or equal to the maximum size of a long
     * word (64-bit word) and we have a place to store the results, then we can
     * proceed.  Otherwise, we have an error.
     */
    if ((val != NULL) && (len <= sizeof(__int64_t)))
    {
	char	*ptr = (char *) val;
	int		ii;

	/*
	 * Set the resulting value to zero.  This will set each of the bytes in
	 * the value to be returned to all zeros.
	 */
	*val = 0;

	/*
	 * Loop through each byte in the put string and store it in the
	 * corresponding byte of the returned value.  This goes from the lowest
	 * to the highest bytes.  NOTE: We can simply cast this because we have
	 * no idea of how long the input string may be.
	 */
	for (ii = 0; ii < len; ii++)
	    ptr[ii] = strVal[ii];
    }
    else
	retVal = 0;

    /*
     * Return the results of this call back to the caller.
     */
    free(strVal);
    return(retVal);
}

/*
 * sdl_offset
 *  This function is called to get the current offset within the current
 *  AGGREGATE definition.
 *
 * Input Parameters:
 *  context:
 *	A pointer to the context structure where we maintain information about
 *	the current parsing.
 *  offsetType:
 *	A value indicating the type of offset to return.  This can be one of the
 *	following values:
 *	    SDL_K_OFF_BYTE_REL	- Relative from the beginning or ORIGIN.
 *	    SDL_K_OFF_BYTE_BEG	- Relative from the beginning.
 *	    SDL_K_OFF_BIT		- bit offset from the beginning or most recent
 *							  element.
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *  0:		if we did not find anything.
 *  >=0:	if we did find something to return.
 */
int sdl_offset(SDL_CONTEXT *context, int offsetType)
{
    int			retVal = 0;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_offset\n", __FILE__, __LINE__);

    /*
     * Return the results of this call back to the caller.
     */
    return(retVal);
}

/*
 * sdl_dimension
 *  This function is called when a DIMENSION modifier is process.  It is called
 *  with 2 values, the lower and higher bounds of the dimension.  For languages
 *  that do not support lower bounds, the range will be appropriately adjusted
 *  to that languages needs, while maintaining the number of requested items.
 *  This section of code makes no assumptions about these two values.  The
 *  index of the entry used is returned back to the caller, which will, in turn
 *  be supplied on the processing of the declaration where the DIMENSION
 *  modifier was specified.
 *
 * Input Parameter:
 *  context:
 *	A pointer to the context structure where we maintain information about
 *	the current parsing.
 *  lbound:
 *	A value indicating the lower bound value for the dimension.
 *  hbound:
 *	A value indicating the upper bound value for the dimension.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  The entry in the dimension array used to store the DIMENSION information
 *  until needed.
 */
int sdl_dimension(SDL_CONTEXT *context, size_t lbound, size_t hbound)
{
    int		retVal = 0;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_dimension\n", __FILE__, __LINE__);

    /*
     * Loop through all the dimension array entries looking for an available
     * slot.
     */
    while ((retVal < SDL_K_MAX_DIMENSIONS) &&
	   (context->dimensions[retVal].inUse == true))
	retVal++;

    /*
     * If we found one, then save the dimension information and set the inUse
     * flag.  Otherwise, we'tt return a value outside the array dimensions to
     * indicate that we ran out of space.
     */
    if (retVal < SDL_K_MAX_DIMENSIONS)
    {
	context->dimensions[retVal].lbound = lbound;
	context->dimensions[retVal].hbound = hbound;
	context->dimensions[retVal].inUse = true;
    }
    else
	retVal = 0;

    /*
     * Return the dimension array location utilized back to the caller.
     */
    return(retVal);
}

/*
 * sdl_add_option
 *  This function is called when it gets to a declaration option.  It will
 *  store the option information for use later, when storing the declaration
 *  in the context.
 *
 * Input Parameters:
 *  context:
 *	A pointer to the context structure where we maintain information about
 *	the current parsing.
 *  option:
 *	An enumeration indicating the option we are being asked to store.
 *  value:
 *  	A 64-bit integer value when the option is an integer.
 *  string:
 *  	A pointer to a string when the option is a string.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  1:	Normal Successful Completion.
 *  0:	An error occurred.
 */
int sdl_add_option(
		SDL_CONTEXT *context,
		SDL_OPTION_TYPE option,
		__int64_t value,
		char *string)
{
    int		retVal = 1;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
    {
	printf(
	    "%s:%d:sdl_add_option(%d, ",
	    __FILE__,
	    __LINE__,
	    option);
	if (string == NULL)
	    printf("%ld)", value);
	else
	    printf("%s)", string);
	printf(" at index %d\n", context->optionsIdx);
    }

    /*
     * The options list is dynamically sized, and is never reduced.  So, if
     * there is not enough room for another option, then reallocate the options
     * list to the next increment
     */
    if (context->optionsIdx >= context->optionsSize)
    {
	size_t	size;

	context->optionsSize += SDL_K_OPTIONS_INCR;
	size = context->optionsSize * sizeof(SDL_OPTION);
	context->options = realloc(context->options, size);
	if (context->options == NULL)
	    retVal = 0;
    }

    /*
     * Add another option, then do so now.
     */
    if (retVal == 1)
    {
	switch (option)
	{

	    /*
	     * We should never get this one.
	     */
	    case None:
		retVal = 0;
		break;

	    /*
	     * The following are just indicated as being present.
	     */
	    case Align:
	    case Common:
	    case Fill:
	    case Enumerate:
	    case Global:
	    case In:
	    case List:
	    case Mask:
	    case NoAlign:
	    case Out:
	    case Optional:
	    case Reference:
	    case Signed:
	    case Typedef:
	    case Value:
	    case Variable:
		context->options[context->optionsIdx++].option = option;
		break;

	    /*
	     * The following are all 64-bit integer values.
	     */
	    case BaseAlign:
	    case Default:
	    case Dimension:
	    case Increment:
	    case Length:
	    case Radix:
	    case ReturnsType:
	    case SubType:
		context->options[context->optionsIdx].option = option;
		context->options[context->optionsIdx++].value = value;
		break;

	    /*
	     * The following are all string values.
	     */
	    case Alias:
	    case Based:
	    case Counter:
	    case Linkage:
	    case Marker:
	    case Named:
	    case Origin:
	    case Prefix:
	    case ReturnsNamed:
	    case Tag:
	    case TypeName:
		context->options[context->optionsIdx].option = option;
		context->options[context->optionsIdx++].string = string;
		break;

	    /*
	     * The following are yet to be determined.
	     */
	    case Parameter:
		break;
	}
    }

    /*
     * Return the results of this call back to the caller.
     */
    return (retVal);
}

/*
 * sdl_precision
 *  This function is called to store the precision and scale information
 *  associated with a DECIMAL declaration.
 *
 * Input Parameters:
 *  context:
 *	A pointer to the context structure where we maintain information about
 *	the current parsing.
 *  precision:
 *	A value indicating the packed decimal precision.
 *  scale:
 *	A value indicating the packed decimal scale.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  1:	Normal Successful Completion.
 *  0:	An error occurred.
 */
int sdl_precision(SDL_CONTEXT *context, __int64_t precision, __int64_t scale)
{
    int		retVal = 1;

    context->precision = precision;
    context->scale = scale;

    /*
     * Return the results of this call back to the caller.
     */
    return (retVal);
}

/*
 * sdl_trim_str
 *  This function is called to remove space characters from a string.  If can
 *  perform 4 kinds of space removal (any one or all at the same time).
 *
 *	SDL_M_LEAD	Remove all leading spaces.
 *	SDL_M_TRAIL	Remove all trailing spaces.
 *	SDL_M_COMPRESS	Convert repeating space characters to a single one.
 *	SDL_M_COLLAPSE	Remove all space characters.
 *	SDL_M_CONVERT	Convert control characters to spaces.
 *	SDL_M_KEEP_NL	Keep the new-line characters.
 *
 * Input Parameters:
 *  str:
 *	A pointer to the string to be trimmed.
 *  type:
 *	A value mask indicating the type of space trimming to be performed.
 *
 * Output Parameters:
 *  str:
 *	A pointer to the updated string (modified in place)
 *
 * Return Values:
 *  None.
 */
void sdl_trim_str(char *str, int type)
{
    int		srcIdx = 0;
    int		destIdx = 0;
    bool	leading = (type & SDL_M_LEAD) != 0;
    bool	trailing = (type & SDL_M_TRAIL) != 0;
    bool	compress = (type & SDL_M_COMPRESS) != 0;
    bool	collapse = (type & SDL_M_COLLAPSE) != 0;
    bool	convert = (type & SDL_M_CONVERT) != 0;
    bool	keepNL = (type & SDL_M_KEEP_NL) != 0;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_trim(0x%08x)\n", __FILE__, __LINE__, type);

    /*
     * If we are to convert control characters to spaces, do so now.  If we are
     * keeping new-line characters, just skip over them.
     */
    if (convert == true)
    {
	while (str[srcIdx] != '\0')
	{
	    if (iscntrl(str[srcIdx]) != 0)
	    {
		if (((keepNL == true) && (str[srcIdx] != '\n')) ||
		    (keepNL == false))
		    str[srcIdx] = ' ';
	    }
	    srcIdx++;
	}
	srcIdx = 0;
    }

    /*
     * If we are stripping leading or compressing multiple or removing all
     * spaces, then we start from the beginning of the string and work out way
     * to the end.  Otherwise, we are probably stripping trailing spaces, in
     * which case, we are stripping from the end of the string forward.
     */
    if ((leading == true) || (compress == true) || (collapse == true))
	while (str[srcIdx] != '\0')
	{

	    /*
	     * Collapsing removes all spaces, so it automatically includes
	     * leading, trailing, and repeated space removal.  It also ignores
	     * keeping new-lines.
	     */
	    if (collapse == true)
	    {
		if (isspace(str[srcIdx]))
		    srcIdx++;
		else
		    str[destIdx++] = str[srcIdx++];
	    }
	    else
	    {
		bool skipCompress = false;

		/*
		 * If we are trimming leading spaces, then we need to skip to
		 * the first non-space character.  We are going to strip any
		 * new-line as well, so don't worry about the setting of that
		 * particular flag.
		 */
		if (leading == true)
		{
		    skipCompress = true;
		    if ((isspace(str[srcIdx])) && (destIdx == 0))
			srcIdx++;
		    else
			str[destIdx++] = str[srcIdx++];
		}

		/*
		 * If we are compressing, then make sure that only one space
		 * character makes it.  Also, if we are keeping new-lines, then
		 * allow one, if present, over all other possible space
		 * characters.
		 */
		if ((compress == true) && (skipCompress == false))
		{

		    /*
		     * Always skip over the first character in the string.
		     */
		    if (destIdx == 0)
			destIdx++;
		    if (srcIdx == 0)
			srcIdx++;

		    /*
		     * If the previously copied character is a space and the
		     * next one to copy is also a space, then unless we are
		     * keeping new-lines, skip over the next character to copy.
		     */
		    if (isspace(str[destIdx - 1]) && isspace(str[srcIdx]))
		    {

			/*
			 * If we are keeping new-lines and the character to be
			 * copied is a new-line, then determine if we should
			 * replace the previous space character with the new-
			 * line character.
			 */
			if ((keepNL == true) && (str[srcIdx] == '\n'))
			{

			    /*
			     * If we previously did not copy in a new-line,
			     * then replace it with the new-line we just hit.
			     */
			    if (str[destIdx - 1] != '\n')
				str[destIdx - 1] = str[srcIdx];
			}
			srcIdx++;
		    }
		    else
			str[destIdx++] = str[srcIdx++];
		}
	    }
	}
    else
	destIdx = strlen(str);

    /*
     * Make sure we terminate the string with a null-character.
     */
    str[destIdx] = '\0';

    /*
     * If we are stripping spaces from the end of the string, then we will just
     * convert the space characters to a null-character.
     */
    if ((trailing == true) && (collapse == false))
    {
	while (isspace(str[--destIdx]))
	    str[destIdx] = '\0';
    }

    /*
     * Return back to the caller.
     */
    return;
}
