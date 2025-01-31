# Medium Intermediate Representation (file mir.h)
  * This document describes MIR itself, API for its creation, and MIR textual representation
  * MIR textual representation is assembler like.  Each directive or insn should be put on a separate line
  * In MIR textual syntax we use
    * `[]` for optional construction
    * `{}` for repeating zero or more times
    * `<>` for some informal construction description or construction already described or will be described
  
## MIR context
  * MIR API code has an implicit state called by MIR context
  * MIR context is represented by data of `MIR_context_t`
  * MIR context is created by function `MIR_context_t MIR_init (void)`
  * Every MIR API function (except for `MIR_init`) requires MIR context passed through the first argument of type `MIR_context_t`
  * You can use MIR functions in different threads without any synchronization
    if they work with different contexts in each thread

## MIR program
   * MIR program consists of MIR **modules**
   * To start work with MIR program, you should first call API function `MIR_init`
   * API function `MIR_finish (MIR_context_t ctx)` should be called last.  It frees all internal data used to work with MIR program and all IR (insns, functions, items, and modules) created in this context
   * API function `MIR_output (MIR_context_t ctx, FILE *f)` outputs MIR textual representation of the program into given file
   * API function `MIR_scan_string (MIR_context_t ctx, const char *str)` reads textual MIR representation given by a string
   * API functions `MIR_write (MIR_context_t ctx, FILE *f)` and
     `MIR_read (MIR_context_t ctx, FILE *f)` outputs and reads
     **binary MIR representation** to/from given file.  There are also
     functions `MIR_write_with_func (MIR_context_t ctx, const int
     (*writer_func) (MIR_context_t, uint8_t))` and `MIR_read_with_func
     (MIR_context_t ctx, const int (*reader_func) (MIR_context_t))` to
     output and read **binary MIR representation** through a function
     given as an argument.  The reader function should return EOF as
     the end of the binary MIR representation, the writer function
     should be return the number of successfully output bytes
     * Binary MIR representation much more compact and faster to read than textual one

## MIR data type
   * MIR program works with the following **data types**:
     * `MIR_T_I8` and `MIR_T_U8` -- signed and unsigned 8-bit integer values
     * `MIR_T_I16` and `MIR_T_U16` -- signed and unsigned 16-bit integer values
     * `MIR_T_I32` and `MIR_T_U32` -- signed and unsigned 32-bit integer values
     * `MIR_T_I64` and `MIR_T_U64` -- signed and unsigned 64-bit integer values
       * ??? signed and unsigned 64-bit integer types in most cases
         are interchangeable as insns themselves decide how to treat
         their value
     * `MIR_T_F` and `MIR_T_D` -- IEEE single and double precision floating point values
     * `MIR_T_LD` - long double values.  It is machine-dependent and can be IEEE double, x86 80-bit FP,
       or IEEE quad precision FP values.  If it is the same as double, the double type will be used instead.
       So please don't expect machine-independence of MIR code working with long double values
     * `MIR_T_P` -- pointer values.  Depending on the target pointer value is actually 32-bit or 64-bit integer value
     * `MIR_T_BLK` .. `MIR_T_BLK + MIR_BLK_NUM - 1` -- block data with given case.  This type can be used only
       for argument of function.  Different case numbers can denote different ways to pass the block data
       on a particular target to implement the target call ABI.  Currently there are 6 block
       types (`MIR_BLK_NUM = 5`)
     * `MIR_T_RBLK` -- return block data.  This type can be used only for argument of function
   * MIR textual representation of the types are correspondingly `i8`,
     `u8`, `i16`, `u16`, `i32`, `u32`, `i64`, `u64`, `f`, `d`, `ld`, `p`,
     and `blk`
   * Function `int MIR_int_type_p (MIR_type_t t)` returns TRUE if given type is an integer one (it includes pointer type too)
   * Function `int MIR_fp_type_p (MIR_type_t t)` returns TRUE if given type is a floating point type
   
## MIR module
  * Module is a high level entity of MIR program
  * Module is created through API function `MIR_module_t MIR_new_module (const char *name)`
  * Module creation is finished by calling API function `MIR_finish_module`
  * You can create only one module at any given time
  * List of all created modules can be gotten by function `DLIST (MIR_module_t) *MIR_get_module_list (MIR_context_t ctx)`
  * MIR module consists of **items**.  There are following **item types** (and function for their creation):
    * **Function**: `MIR_func_item`
    * **Import**: `MIR_import_item` (`MIR_item_t MIR_new_import (MIR_context_t ctx, const char *name)`)
    * **Export**: `MIR_export_item` (`MIR_item_t MIR_new_export (MIR_context_t ctx, const char *name)`)
    * **Forward declaration**: `MIR_forward_item` (`MIR_item_t MIR_new_forward (MIR_context_t ctx, const char *name)`)
    * **Prototype**: `MIR_proto_item` (`MIR_new_proto_arr`, `MIR_new_proto`, `MIR_new_vararg_proto_arr`,
      `MIR_new_vararg_proto` analogous to `MIR_new_func_arr`, `MIR_new_func`, `MIR_new_vararg_func_arr` and
      `MIR_new_vararg_func` -- see below).  The only difference is that
      two or more prototype argument names can be the same
    * **Data**: `MIR_data_item` with optional name
      (`MIR_item_t MIR_new_data (MIR_context_t ctx, const char *name, MIR_type_t el_type, size_t nel, const void *els)`
       or `MIR_item_t MIR_new_string_data (MIR_context_t ctx, const char *name, MIR_str_t str)`)
    * **Reference data**: `MIR_ref_data_item` with optional name
      (`MIR_item_t MIR_new_ref_data (MIR_context_t ctx, const char *name, MIR_item_t item, int64_t disp)`
      * The address of the item after linking plus `disp` is used to initialize the data
    * **Expression Data**: `MIR_expr_data_item` with optional name
      (`MIR_item_t MIR_new_expr_data (MIR_context_t ctx, const char *name, MIR_item_func_item)`)
      * Not all MIR functions can be used for expression data.  The expression function should have
        only one result, have no arguments, not use any call or any instruction with memory
      * The expression function is called during linking and its result is used to initialize the data
    * **Memory segment**: `MIR_bss_item` with optional name (`MIR_item_t MIR_new_bss (MIR_context_t ctx, const char *name, size_t len)`)
  * Long double data item is changed to double one, if long double coincides with double for given target or ABI
  * Names of MIR functions, imports, and prototypes should be unique in a module
  * API functions `MIR_output_item (MIR_context_t ctx, FILE *f, MIR_item_t item)`
    and `MIR_output_module (MIR_context_t ctx, FILE *f, MIR_module_t module)` output item or module
    textual representation into given file
  * MIR text module syntax looks the following:
```
    <module name>: module
                   {<module item>}
                   endmodule
```

## MIR function
  * Function is an module item
  * Function has a **frame**, a stack memory reserved for each function invocation
  * Function has **local variables** (sometimes called **registers**), a part of which are **arguments**
    * A variable should have an unique name in the function
    * A variable is represented by a structure of type `MIR_var_t`
      * The structure contains variable name and its type
      * The structure contains also type size for variable of block types (`MIR_T_BLK`..`MIR_T_BLK + MIR_BLK_NUM - 1`)
        or `MIR_T_RBLK` type
  * MIR function with its arguments is created through API function `MIR_item_t MIR_new_func (MIR_context_t ctx, const
    char *name, size_t nres, MIR_type_t *res_types, size_t nargs, ...)`
    or function `MIR_item_t MIR_new_func_arr (MIR_context_t ctx, const char *name, size_t nres, MIR_type_t *res_types, size_t nargs, MIR_var_t *arg_vars)`
    * Argument variables can be any type
      * This type only denotes how the argument value is passed
      * Any integer type argument variable has actually type `MIR_T_I64`
  * MIR functions with variable number of arguments are created through API functions
    `MIR_item_t MIR_new_vararg_func (MIR_context_t ctx, const char *name, size_t nres, MIR_type_t *res_types, size_t nargs, ...)`
    or function `MIR_item_t MIR_new_vararg_func_arr (MIR_context_t ctx, const char *name, size_t nres, MIR_type_t *res_types, size_t nargs, MIR_var_t *arg_vars)`
    * `nargs` and `arg_vars` define only fixed arguments
    * MIR functions can have more one result but possible number of results
      and combination of their types are machine-defined.  For example, for x86-64
      the function can have up to six results and return two integer
      values, two float or double values, and two long double values
      in any combination
  * MIR function creation is finished by calling API function `MIR_finish_func (MIR_context_t ctx)`
  * You can create only one MIR function at any given time
  * MIR text function syntax looks the following (arg-var always has a name besides type):
```
    <function name>: func {<result type>, } [ arg-var {, <arg-var> } [, ...]]
                     {<insn>}
                     endfun
```
    * Textual presentation of block type argument in `func` has form `blk:<size>(<var_name>)`.
      The corresponding argument in `call` insn should have analogous form
      `blk:<the same size>(<local var name containing address of passed block data>)`
    * Block data are passed by value.  How they are exactly passed is machine-defined (please read files mir-<target>.c):
      * they can be passed on stack, or (partially) in registers, or by address
  * Non-argument function variables are created through API function
    `MIR_reg_t MIR_new_func_reg (MIR_context_t ctx, MIR_func_t func, MIR_type_t type, const char *name)`
    * The only permitted integer type for the variable is `MIR_T_I64` (or MIR_T_U64???)
    * Names in form `t<number>` can not be used as they are fixed for internal purposes
    * You can create function variables even after finishing the
      function creation.  This can be used to modify function insns,
      e.g. for optimizations
  * Non-argument variable declaration syntax in MIR textual representation looks the following:
```
    local [ <var type>:<var name> {, <var type>:<var name>} ]
```
  * In MIR textual representation variable should be defined through `local` before its use
    
## MIR insn operands
  * MIR insns work with operands
  * There are following operands:
    * Signed or unsigned **64-bit integer value operands** created through API functions
      `MIR_op_t MIR_new_int_op (MIR_context_t ctx, int64_t v)` and `MIR_op_t MIR_new_uint_op (MIR_context_t ctx, uint64_t v)`
      * In MIR text they are represented the same way as C integer numbers (e.g. octal, decimal, hexadecimal ones)
    * **Float, double or long double value operands** created through API functions `MIR_op_t MIR_new_float_op (MIR_context_t ctx, float v)`,
      `MIR_op_t MIR_new_double_op (MIR_context_t ctx, double v)`,
      and `MIR_op_t MIR_new_ldouble_op (MIR_context_t ctx, long double v)`.
      Long double operand is changed to double one when long double coincides with double for given target or ABI
      * In MIR text they are represented the same way as C floating point numbers
    * **String operands** created through API functions `MIR_op_t MIR_new_str_op (MIR_context_t ctx, MIR_str_t str)`
      * In MIR text they are represented by `typedef struct MIR_str {size_t len; const char *s;} MIR_str_t`
      * Strings for each operand are put into memory (which can be modified) and the memory address actually presents the string
    * **Label operand** created through API function `MIR_op_t MIR_new_label_op (MIR_context_t ctx, MIR_label_t label)`
      * Here `label` is a special insn created by API function `MIR_insn_t MIR_new_label (MIR_context_t ctx)`
      * In MIR text, they are represented by unique label name
    * **Reference operands** created through API function `MIR_op_t MIR_new_ref_op (MIR_context_t ctx, MIR_item_t item)`
      * In MIR text, they are represented by the corresponding item name
    * **Register (variable) operands** created through API function `MIR_op_t MIR_new_reg_op (MIR_context_t ctx, MIR_reg_t reg)`
      * In MIR text they are represented by the corresponding variable name
      * Value of type `MIR_reg_t` is returned by function `MIR_new_func_reg`
        or can be gotten by function `MIR_reg_t MIR_reg (MIR_context_t ctx, const char *reg_name, MIR_func_t func)`, e.g. for argument-variables
    * **Memory operands** consists of type, displacement, base
      register, index register and index scale.  Memory operand is
      created through API function `MIR_op_t MIR_new_mem_op (MIR_context_t ctx, MIR_type_t type,
      MIR_disp_t disp, MIR_reg_t base, MIR_reg_t index, MIR_scale_t
      scale)`
      * The arguments define address of memory as `disp + base + index * scale`
      * Integer type input memory is transformed to 64-bit integer value with sign or zero extension
        depending on signedness of the type
      * result 64-bit integer value is truncated to integer memory type
      * Memory operand has the following syntax in MIR text (absent displacement means zero one,
        absent scale means one, scale should be 1, 2, 4, or 8):
      
```
	  <type>: <disp>
	  <type>: [<disp>] (<base reg> [, <index reg> [, <scale> ]])
```
  * API function `MIR_output_op (MIR_context_t ctx, FILE *f, MIR_op_t op, MIR_func_t func)` outputs the operand
    textual representation into given file
        

## MIR insns
  * All MIR insns (but call or ret one) expects fixed number of operands
  * Most MIR insns are 3-operand insns: two inputs and one output
  * In majority cases **the first insn operand** describes where the insn result (if any) will be placed
  * Only register or memory operand can be insn output (result) operand
  * MIR insn can be created through API functions `MIR_insn_t MIR_new_insn (MIR_context_t ctx, MIR_insn_code_t code, ...)`
    and `MIR_insn_t MIR_new_insn_arr (MIR_context_t ctx, MIR_insn_code_t code, size_t nops, MIR_op_t *ops)`
    * Number of operands and their types should be what is expected by the insn being created
    * You can not use `MIR_new_insn` for the creation of call and ret insns as these insns have a variable number of operands.
      To create such insns you should use `MIR_new_insn_arr` or special functions
      `MIR_insn_t MIR_new_call_insn (MIR_context_t ctx, size_t nops, ...)` and `MIR_insn_t MIR_new_ret_insn (MIR_context_t ctx, size_t nops, ...)`
    * Long double insns are changed by double ones if long double coincides with double for given target or ABI
  * You can get insn name and number of insn operands through API functions
    `const char *MIR_insn_name (MIR_context_t ctx, MIR_insn_code_t code)` and `size_t MIR_insn_nops (MIR_context_t ctx, MIR_insn_t insn)`
  * You can add a created insn at the beginning or end of function insn list through API functions
    `MIR_prepend_insn (MIR_context_t ctx, MIR_item_t func, MIR_insn_t insn)` and `MIR_append_insn (MIR_context_t ctx, MIR_item_t func, MIR_insn_t insn)`
  * You can insert a created insn in the middle of function insn list through API functions
    `MIR_insert_insn_after (MIR_context_t ctx, MIR_item_t func, MIR_insn_t after, MIR_insn_t insn)` and
    `MIR_insert_insn_before (MIR_context_t ctx, MIR_item_t func, MIR_insn_t before, MIR_insn_t insn)`
    * The insn `after` and `before` should be already in the list
  * You can remove insn from the function list through API function `MIR_remove_insn (MIR_context_t ctx, MIR_item_t func, MIR_insn_t insn)`
  * The insn should be not inserted in the list if it is already there
  * The insn should be not removed form the list if it is not there
  * API function `MIR_output_insn (MIR_context_t ctx, FILE *f, MIR_insn_t insn, MIR_func_t func, int newline_p)` outputs the insn
    textual representation into given file with a newline at the end depending on value of `newline_p`
  * Insn has the following syntax in MIR text:
```
	  {<label name>:} [<insn name> <operand> {, <operand>}]
```
  * More one insn can be put on the same line by separating the insns by `;`

### MIR move insns
  * There are following MIR move insns:

    | Insn Code               | Nops |   Description                                          |
    |-------------------------|-----:|--------------------------------------------------------|
    | `MIR_MOV`               | 2    | move 64-bit integer values                             |
    | `MIR_FMOV`              | 2    | move **single precision** floating point values        |
    | `MIR_DMOV`              | 2    | move **double precision** floating point values        |
    | `MIR_LDMOV`             | 2    | move **long double** floating point values             |

### MIR integer insns
  * If insn has suffix `S` in insn name, the insn works with lower 32-bit part of 64-bit integer value
  * The higher part of 32-bit insn result is undefined
  * If insn has prefix `U` in insn name, the insn treats integer as unsigned integers
  * Some insns has no unsigned variant as MIR is oriented to CPUs with two complement integer arithmetic
    (the huge majority of all CPUs)
  
    | Insn Code               | Nops |   Description                                          |
    |-------------------------|-----:|--------------------------------------------------------|
    | `MIR_EXT8`              | 2    | **sign** extension of lower **8 bit** input part       |
    | `MIR_UEXT8`             | 2    | **zero** extension of lower **8 bit** input part       |
    | `MIR_EXT16`             | 2    | **sign** extension of lower **16 bit** input part      |
    | `MIR_UEXT16`            | 2    | **zero** extension of lower **16 bit** input part      |
    | `MIR_EXT32`             | 2    | **sign** extension of lower **32 bit** input part      |
    | `MIR_UEXT32`            | 2    | **zero** extension of lower **32 bit** input part      |
    |                         |      |                                                        |
    | `MIR_NEG`               | 2    | changing sign of **64-bit* integer value               |
    | `MIR_NEGS`              | 2    | changing sign of **32-bit* integer value               |
    |                         |      |                                                        |
    | `MIR_ADD`, `MIR_SUB`    | 3    | **64-bit** integer addition and subtraction            |
    | `MIR_ADDS`, `MIR_SUBS`  | 3    | **32-bit** integer addition and subtraction            |
    | `MIR_MUL`, `MIR_DIV`    | 3    | **64-bit signed**  multiplication and divison          |
    | `MIR_UMUL`, `MIR_UDIV`  | 3    | **64-bit unsigned** integer multiplication and divison |
    | `MIR_MULS`, `MIR_DIVS`  | 3    | **32-bit signed**  multiplication and divison          |
    | `MIR_UMULS`, `MIR_UDIVS`| 3    | **32-bit unsigned** integer multiplication and divison |
    | `MIR_MOD`               | 3    | **64-bit signed**  modulo operation                    |
    | `MIR_UMOD`              | 3    | **64-bit unsigned** integer modulo operation           |
    | `MIR_MODS`              | 3    | **32-bit signed**  modulo operation                    |
    | `MIR_UMODS`             | 3    | **32-bit unsigned** integer modulo operation           |
    |                         |      |                                                        |
    | `MIR_AND`, `MIR_OR`     | 3    | **64-bit** integer bitwise AND and OR                  |
    | `MIR_ANDS`, `MIR_ORS`   | 3    | **32-bit** integer bitwise AND and OR                  |
    | `MIR_XOR`               | 3    | **64-bit** integer bitwise XOR                         |
    | `MIR_XORS`              | 3    | **32-bit** integer bitwise XOR                         |
    |                         |      |                                                        |
    | `MIR_LSH`               | 3    | **64-bit** integer left shift                          |
    | `MIR_LSHS`              | 3    | **32-bit** integer left shift                          |
    | `MIR_RSH`               | 3    | **64-bit** integer right shift with **sign** extension |
    | `MIR_RSHS`              | 3    | **32-bit** integer right shift with **sign** extension |
    | `MIR_URSH`              | 3    | **64-bit** integer right shift with **zero** extension |
    | `MIR_URSHS`             | 3    | **32-bit** integer right shift with **zero** extension |
    |                         |      |                                                        |
    | `MIR_EQ`, `MIR_NE`      | 3    | equality/inequality of **64-bit** integers             |
    | `MIR_EQS`, `MIR_NES`    | 3    | equality/inequality of **32-bit** integers             |
    | `MIR_LT`, `MIR_LE`      | 3    | **64-bit signed** less than/less than or equal         |
    | `MIR_ULT`, `MIR_ULE`    | 3    | **64-bit unsigned** less than/less than or equal       |
    | `MIR_LTS`, `MIR_LES`    | 3    | **32-bit signed** less than/less than or equal         |
    | `MIR_ULTS`, `MIR_ULES`  | 3    | **32-bit unsigned** less than/less than or equal       |
    | `MIR_GT`, `MIR_GE`      | 3    | **64-bit signed** greater than/greater than or equal   |
    | `MIR_UGT`, `MIR_UGE`    | 3    | **64-bit unsigned** greater than/greater than or equal |
    | `MIR_GTS`, `MIR_GES`    | 3    | **32-bit signed** greater than/greater than or equal   |
    | `MIR_UGTS`, `MIR_UGES`  | 3    | **32-bit unsigned** greater than/greater than or equal |

### MIR floating point insns
  * If insn has prefix `F` in insn name, the insn is single precision float point insn.  Its operands should have `MIR_T_F` type
  * If insn has prefix `D` in insn name, the insn is double precision float point insn.  Its operands should have `MIR_T_D` type
  * Otherwise, insn has prefix `LD` in insn name and the insn is a long double insn.
    Its operands should have `MIR_T_LD` type.
  * The result of comparison insn is a 64-bit integer value, so the result operand should be of integer type
  
    | Insn Code                            | Nops |   Description                                                   |
    |--------------------------------------|-----:|-----------------------------------------------------------------|
    | `MIR_F2I`, `MIR_D2I`, `MIR_LD2I`     | 2    | transforming floating point value into 64-bit integer           |
    | `MIR_F2D`                            | 2    | transforming single to double precision FP value                |
    | `MIR_F2LD`                           | 2    | transforming single precision to long double FP value           |
    | `MIR_D2F`                            | 2    | transforming double to single precision FP value                |
    | `MIR_D2LD`                           | 2    | transforming double precision to long double FP value           |
    | `MIR_LD2F`                           | 2    | transforming long double to single precision FP value           |
    | `MIR_LD2D`                           | 2    | transforming long double to double precision FP value           |
    | `MIR_I2F`, `MIR_I2D`, `MIR_I2LD`     | 2    | transforming 64-bit integer into a floating point value         |
    | `MIR_UI2F`, `MIR_UI2D`, `MIR_UI2LD`  | 2    | transforming unsigned 64-bit integer into a floating point value|
    | `MIR_FNEG`, `MIR_DNEG`, `MIR_LDNEG`  | 2    | changing sign of floating point value                           |
    | `MIR_FADD`, `MIR_FSUB`               | 3    | **single** precision addition and subtraction                   |
    | `MIR_DADD`, `MIR_DSUB`               | 3    | **double** precision addition and subtraction                   |
    | `MIR_LDADD`, `MIR_LDSUB`             | 3    | **long double** addition and subtraction                        |
    | `MIR_FMUL`, `MIR_FDIV`               | 3    | **single** precision multiplication and divison                 |
    | `MIR_DMUL`, `MIR_DDIV`               | 3    | **double** precision multiplication and divison                 |
    | `MIR_LDMUL`, `MIR_LDDIV`             | 3    | **long double** multiplication and divison                      |
    | `MIR_FEQ`, `MIR_FNE`                 | 3    | equality/inequality of **single** precision values              |
    | `MIR_DEQ`, `MIR_DNE`                 | 3    | equality/inequality of **double** precision values              |
    | `MIR_LDEQ`, `MIR_LDNE`               | 3    | equality/inequality of **long double** values                   |
    | `MIR_FLT`, `MIR_FLE`                 | 3    | **single** precision less than/less than or equal               |
    | `MIR_DLT`, `MIR_DLE`                 | 3    | **double** precision less than/less than or equal               |
    | `MIR_LDLT`, `MIR_LDLE`               | 3    | **long double** less than/less than or equal                    |
    | `MIR_FGT`, `MIR_FGE`                 | 3    | **single** precision greater than/greater than or equal         |
    | `MIR_DGT`, `MIR_DGE`                 | 3    | **double** precision greater than/greater than or equal         |
    | `MIR_LDGT`, `MIR_LDGE`               | 3    | **long double** greater than/greater than or equal              |

### MIR branch insns
  * The first operand of the insn should be label

    | Insn Code               | Nops |   Description                                                 |
    |-------------------------|-----:|---------------------------------------------------------------|
    | `MIR_JMP`               | 1    | unconditional jump to the label                               |
    | `MIR_BT`                | 2    | jump to the label when 2nd **64-bit** operand is **nonzero**  |
    | `MIR_BTS`               | 2    | jump to the label when 2nd **32-bit** operand is **nonzero**  |
    | `MIR_BF`                | 2    | jump to the label when 2nd **64-bit** operand is **zero**     |
    | `MIR_BFS`               | 2    | jump to the label when 2nd **32-bit** operand is **zero**     |

### MIR switch insn
  * The first operand of `MIR_SWITCH` insn should have an integer value from 0 to `N - 1` inclusive
  * The rest operands should be `N` labels, where `N > 0`
  * Execution of the insn will be an jump on the label corresponding to the first operand value
  * If the first operand value is out of the range of permitted values, the execution result is undefined

### MIR integer comparison and branch insn
  * The first operand of the insn should be label.  Label will be the next executed insn if the result of comparison is non-zero

    | Insn Code               | Nops |   Description                                                 |
    |-------------------------|-----:|---------------------------------------------------------------|
    | `MIR_BEQ`, `MIR_BNE`    | 3    | jump on **64-bit** equality/inequality                        |
    | `MIR_BEQS`, `MIR_BNES`  | 3    | jump on **32-bit** equality/inequality                        |
    | `MIR_BLT`, `MIR_BLE`    | 3    | jump on **signed 64-bit** less than/less than or equal        |
    | `MIR_UBLT`, `MIR_UBLE`  | 3    | jump on **unsigned 64-bit** less than/less than or equal      |
    | `MIR_BLTS`, `MIR_BLES`  | 3    | jump on **signed 32-bit** less than/less than or equal        |
    | `MIR_UBLTS`, `MIR_UBLES`| 3    | jump on **unsigned 32-bit** less than/less than or equal      |
    | `MIR_BGT`, `MIR_BGE`    | 3    | jump on **signed 64-bit** greater than/greater than or equal  |
    | `MIR_UBGT`, `MIR_UBGE`  | 3    | jump on **unsigned 64-bit** greater than/greater than or equal|
    | `MIR_BGTS`, `MIR_BGES`  | 3    | jump on **signed 32-bit** greater than/greater than or equal  |
    | `MIR_UBGTS`, `MIR_UBLES`| 3    | jump on **unsigned 32-bit** greater than/greater than or equal|

### MIR floating point comparison and branch insn
  * The first operand of the insn should be label.  Label will be the next executed insn if the result of comparison is non-zero
  * See comparison semantics in the corresponding comparison insns

    | Insn Code                 | Nops |   Description                                                  |
    |---------------------------|-----:|----------------------------------------------------------------|
    | `MIR_FBEQ`, `MIR_FBNE`    | 3    | jump on **single** precision equality/inequality               |
    | `MIR_DBEQ`, `MIR_DBNE`    | 3    | jump on **double** precision equality/inequality               |
    | `MIR_LDBEQ`, `MIR_LDBNE`  | 3    | jump on **long double** equality/inequality                    |
    | `MIR_FBLT`, `MIR_FBLE`    | 3    | jump on **single** precision less than/less than or equal      |
    | `MIR_DBLT`, `MIR_DBLE`    | 3    | jump on **double** precision less than/less than or equal      |
    | `MIR_LDBLT`, `MIR_LDBLE`  | 3    | jump on **long double** less than/less than or equal           |
    | `MIR_FBGT`, `MIR_FBGE`    | 3    | jump on **single** precision greater than/greater than or equal|
    | `MIR_DBGT`, `MIR_DBGE`    | 3    | jump on **double** precision greater than/less/ than or equal  |
    | `MIR_LDBGT`, `MIR_LDBGE`  | 3    | jump on **long double** greater than/less/ than or equal       |

### MIR return insn
  * Return insn has zero or more operands
  * Return insn operands should correspond to return types of the function
  * 64-bit integer value is truncated to the corresponding function return type first
  * The return values will be the function call values

### MIR_CALL insn
  * The insn has variable number of operands
  * The first operand is a prototype reference operand
  * The second operand is a called function address
    * The prototype should correspond MIR function definition if function address represents a MIR function
    * The prototype should correspond C function definition if the address is C function address
  * If the prototype has *N* return types, the next *N* operands are
    output operands which will contain the result values of the function
    call
  * The subsequent operands are arguments.  Their types and number and should be the same as in the prototype
    * Integer arguments are truncated according to integer prototype argument type
  
### MIR_INLINE insn
  * This insn is analogous to `MIR_CALL` but after linking this insn
    will be changed by inlined function body if it is possible
  * Calls of vararg functions are never inlined
  
### MIR_ALLOCA insn
  * Reserve memory on the stack whose size is given as the 2nd operand and assign the memory address to the 1st operand
  * The reserved memory will be aligned according target ABI

### MIR_BSTART and MIR_BEND insns
  * MIR users can use them implement blocks with automatic
    deallocation of memory allocated by `MIR_ALLOCA` inside the
    blocks.  But mostly these insns are used to implement call
    inlining of functions using alloca
  * The both insns use one operand
  * The first insn saves the stack pointer in the operand
  * The second insn restores stack pointer from the operand
  
### MIR_VA_START, MIR_VA_ARG, MIR_VA_BLOCK_ARG, and MIR_VA_END insns
  * These insns are only for variable number arguments functions
  * `MIR_VA_START` and `MIR_VA_END` have one input operand, an address
    of va_list structure (see C stdarg.h for more details).  Unlike C
    va_start, MIR_VA_START just takes one parameter
  * `MIR_VA_ARG` takes va_list and any memory operand and returns
    address of the next argument in the 1st insn operand.  The memory
    operand type defines the type of the argument
  * `MIR_VA_BLOCK_ARG` takes result address, va_list address, integer operand (size),
    and block type (case) number and moves the next argument passed as block of given
    size and type to the result address
  * va_list operand can be memory with undefined type.  In this case
    address of the va_list is not in the memory but is the
    memory address
  
## MIR API example
  * The following code on C creates MIR analog of C code
    `int64_t loop (int64_t arg1) {int64_t count = 0; while (count < arg1) count++; return count;}`
```c
  MIR_module_t m = MIR_new_module (ctx, "m");
  MIR_item_t func = MIR_new_func (ctx, "loop", MIR_T_I64, 1, MIR_T_I64, "arg1");
  MIR_reg_t COUNT = MIR_new_func_reg (ctx, func->u.func, MIR_T_I64, "count");
  MIR_reg_t ARG1 = MIR_reg (ctx, "arg1", func->u.func);
  MIR_label_t fin = MIR_new_label (ctx), cont = MIR_new_label (ctx);

  MIR_append_insn (ctx, func, MIR_new_insn (ctx, MIR_MOV, MIR_new_reg_op (ctx, COUNT),
                                            MIR_new_int_op (ctx, 0)));
  MIR_append_insn (ctx, func, MIR_new_insn (ctx, MIR_BGE, MIR_new_label_op (ctx, fin),
                                            MIR_new_reg_op (ctx, COUNT), MIR_new_reg_op (ctx, ARG1)));
  MIR_append_insn (ctx, func, cont);
  MIR_append_insn (ctx, func, MIR_new_insn (ctx, MIR_ADD, MIR_new_reg_op (ctx, COUNT),
                                            MIR_new_reg_op (ctx, COUNT), MIR_new_int_op (ctx, 1)));
  MIR_append_insn (ctx, func, MIR_new_insn (ctx, MIR_BLT, MIR_new_label_op (ctx, cont),
                                            MIR_new_reg_op (ctx, COUNT), MIR_new_reg_op (ctx, ARG1)));
  MIR_append_insn (ctx, func, fin);
  MIR_append_insn (ctx, func, MIR_new_ret_insn (ctx, 1, MIR_new_reg_op (ctx, COUNT)));
  MIR_finish_func (ctx);
  MIR_finish_module (ctx);
```

## MIR text examples

  * Sieve of Eratosthenes:

```mir
m_sieve:  module
          export sieve
sieve:    func i32, i32:N
          local i64:iter, i64:count, i64:i, i64:k, i64:prime, i64:temp, i64:flags
          alloca flags, 819000
          mov iter, 0
loop:     bge fin, iter, N
          mov count, 0;  mov i, 0
loop2:    bge fin2, i, 819000
          mov u8:(flags, i), 1;  add i, i, 1
          jmp loop2
fin2:     mov i, 0
loop3:    bge fin3, i, 819000
          beq cont3, u8:(flags,i), 0
          add temp, i, i;  add prime, temp, 3;  add k, i, prime
loop4:    bge fin4, k, 819000
          mov u8:(flags, k), 0;  add k, k, prime
          jmp loop4
fin4:     add count, count, 1
cont3:    add i, i, 1
          jmp loop3
fin3:     add iter, iter, 1
          jmp loop
fin:      rets count
          endfunc
          endmodule
m_ex100:  module
format:   string "sieve (10) = %d\n"
p_printf: proto p:fmt, i32:v
p_seive:  proto i32, i32:iter
          export ex100
          import sieve, printf
ex100:    func v
          local i64:r
          call p_sieve, sieve, r, 100
          call p_printf, printf, format, r
          endfunc
          endmodule
```

  * Example of block arguments and `va_stack_arg`
  
```mir
m0:       module
f_p:	  proto i64, 16:blk(a), ...
f:	  func i64, 16:blk(a), ...
          local i64:r, i64:va, i64:a2
	  alloca va, 32  # allocate enough space va_list
	  va_start va
	  va_stack_arg a2, va, 16 # get address of the 2nd blk arg
	  add r, i64:0(a), i64:8(a2)
	  ret r
main:	  func
	  local i64:a, i64:r
	  alloca a, 16
          mov i64:0(a), 42
          mov i64:8(a), 24
	  call f_p, f, r, blk:16(a), blk:16(a)
	  ret r
	  endfunc
          endmodule
```

## Other MIR API functions
  * MIR API can find a lot of errors.  They are reported through a
    error function of type `void (*MIR_error_func_t) (MIR_context ctx, MIR_error_type_t
    error_type, const char *message)`.  The function is considered to
    never return.  To see all error types, please look at the
    definition of error type `MIR_error_type_t` in file mir.h
  * You can get and set up the current error function through API
    functions `MIR_error_func_t MIR_get_error_func (MIR_context ctx)` and `MIR_set_error_func
    (MIR_context ctx, MIR_error_func_t func)`.
    * The default error function prints the message into stderr and call `exit (1)`
  * MIR is pretty flexible and can describe complex insns, e.g. insns
    whose all operands are memory.  Sometimes you need a very simple
    form of MIR representation.  During load of module all its functions are simplified as much
    as possible by adding new insns and registers resulting in a form in which:
    * immediate, memory, reference operands can be used only in move insns
    * memory have only base register (no displacement and index register)
    * string and float immediate operands (if `mem_float_p`) are changed onto
      references for new string and data items
  * Before execution of MIR code (through interpreter or machine code generated by JIT),
    you need to load and link it
    * You can load MIR module through API function `MIR_load_module
      (MIR_context ctx, MIR_module_t m)`.  The function simplifies module code.
      It also allocates the module data/bss
      and makes visible the exported module items to other module
      during subsequent linking.  There is a guarantee that the
      different data/bss items will be in adjacent memory if the
      data/bss items go one after another and all the data/bss items
      except the first one are anonymous (it means they have no name).
      Such adjacent data/bss items are called a **section**.
      Alignment of the section is malloc alignment.  There are no any
      memory space between data/bss in the section.  If you need to
      provide necessary alignment of a data/bss in the section you
      should do it yourself by putting additional anonymous data/bss
      before given data/bss if it is necessary.  BSS memory is
      initialized by zero and data memory is initialized by the
      corresponding data.  If there is already an exported item with
      the same name, it will be not visible for linking anymore.  Such
      visibility mechanism permits usage of different versions of the
      same function
    * Reference data are initialized not during loading but during linking after
      the referenced item address is known.  The address is used for the data
      initialization
    * Expression data are also initialized not during loading but during linking after
      all addresses are known.  The expression function is evaluated by the interpreter
      and its evaluation result is used for the data initialization.  For example, if
      you need to initialize data by item address plus offset you should use
      an expression data
    * MIR permits to use imported items not implemented in MIR, for
      example to use C standard function `strcmp`.  You need to inform
      MIR about it.  API function `MIR_load_external (MIR_context ctx, const char
      *name, void *addr)` informs that imported items with given name
      have given address (e.g. C function address or data)
    * Imports/exports of modules loaded since the last link can be
      linked through API function `MIR_link (MIR_context ctx, void (*set_interface) (MIR_item_t item),
      void * (*import_resolver) (const char *))`
    * `MIR_link` function inlines most `MIR_INLINE` calls
    * `MIR_link` function also sets up call interface
      * If you pass `MIR_set_interp_interface` to `MIR_link`, then
        called functions from MIR code will be interpreted
      * If you pass `MIR_set_gen_interface` to `MIR_link`, then
        MIR-generator will generate machine code for all loaded MIR
        functions and called functions from MIR code will execute the
        machine code
      * If you pass `MIR_set_lazy_gen_interface` to `MIR_link`, then
        MIR-generator will generate machine code only on the first
        function call and called functions from MIR code will execute
        the machine code
      * If you pass non-null `import_resolver` function, it will be
        called for defining address for import without definition.
        The function get the import name and return the address which
        will be used for the import item.  This function can be useful
        for searching `dlopen` library symbols when use of
        MIR_load_external is not convenient

# MIR code execution
  * Linked MIR code can be executed by an **interpreter** or machine code generated by **MIR generator**

# MIR code interpretation
  * The interpreter is an obligatory part of MIR API because it can be used during linking
  * The interpreter is automatically initialized and finished with MIR API initialization and finishing
  * The interpreter works with values represented by type `MIR_val_t` which is union
    `union {..., int64_t i; uint64_t u; float f; double d; long double d;}`
  * You can execute a MIR function code by API functions `void
    MIR_interp (MIR_context ctx, MIR_item_t func_item, MIR_val_t *results, size_t nargs, ...)` and
    `void MIR_interp_arr (MIR_context ctx, MIR_item_t func_item, MIR_val_t *results, size_t nargs,
    MIR_val_t *vals)`
    * The function results are returned through parameter `results`.  You should pass
      a container of enough size to return all function results.
  * You can execute a MIR function code also through C function call
    mechanism.  First you need to setup the C function interface
    through API function `MIR_set_interp_interface (MIR_context ctx, MIR_item_t
    func_item)`.  After that you can `func_item->addr` to call the
    MIR function as usual C function
    * C function interface is implemented by generation of machine
      code specialized for MIR function.  Therefore the interface
      works only on the same targets as MIR generator

# MIR generator (file mir-gen.h)
  * Before use of MIR generator for given context you should initialize it by API function
    `MIR_gen_init (MIR_context ctx, int gens_num)`.  `gens_num` defines how many generator instances you need.
    Each generator instance can be used in a different thread to compile different MIR functions from the same context.
    If you pass a negative or zero number `gens_num`, it will have the same effect as value `1`
  * API function `MIR_gen_finish (MIR_context ctx)` frees all internal generator data (and its instances) for the context.
    If you want to generate code for the context again after the `MIR_gen_finish` call, you should call
    `MIR_gen_init` again first
  * API function `void *MIR_gen (MIR_context ctx, int gen_num, MIR_item_t func_item)` generates machine code
    of given MIR function in generator instance `gen_num` and returns an address to call it.  You can call
    the code as usual C function by using this address as the called function address.
    `gen_num`  should be a number in the range `0` .. `gens_num - 1` from corresponding `MIR_gen_init`
  * API function `void MIR_gen_set_debug_file (MIR_context_t ctx, int gen_num, FILE *f)` sets up MIR generator
    debug file to `f` for generator instance `gen_num`.
    If it is not NULL a debugging and optimization information will be output to the file according to the
    current generator debug level.  It is useful mostly for MIR developers
  * API function `void MIR_gen_set_debug_level (MIR_context_t ctx, int gen_num, level)` sets up MIR generator
    debug level to `level` for generator instance `gen_num`.  The default level value is maximum possible level
    for printing information as much as possible.  Negative level results in no output.  The function is useful
    mostly for MIR developers
  * API function `void MIR_gen_set_optimize_level (MIR_context_t ctx, int gen_num, unsigned int level)` sets up optimization
    level for MIR generator instance `gen_num`:
    * `0` means only register allocator and machine code generator work
    * `1` means additional code selection task.  On this level MIR generator creates more compact and faster
      code than on zero level with practically on the same speed
    * `2` means additionally common sub-expression elimination and sparse conditional constant propagation.
       This is a default level.  This level is valuable if you generate bad input MIR code with a lot redundancy
       and constants.  The generation speed on level `1` is about 50% faster than on level `2`
    * `3` means additionally register renaming and loop invariant code motion.  The generation speed
      on level `2` is about 50% faster than on level `3`
