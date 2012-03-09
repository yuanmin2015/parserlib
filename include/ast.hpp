#ifndef AST_HPP
#define AST_HPP


#include <cassert>
#include "parser.hpp"


namespace parserlib {


class ast_node;


/** type of AST node stack.
 */
typedef std::vector<ast_node *> ast_stack; 


/** Base class for AST nodes.
 */
class ast_node {
public:
    ///destructor.
    virtual ~ast_node() {}
    
    /** interface for filling the contents of the node
        from a node stack.
        @param b begin position in the source.
        @param e end position in the source.
        @param st stack.
     */
    virtual void construct(const pos &b, const pos &e, ast_stack &st) = 0;
}; 


class ast_member;


/** type of ast member vector.
 */
typedef std::vector<ast_member *> ast_member_vector; 


/** base class for AST nodes with children.
 */
class ast_container : public ast_node {
public:
    /** sets the container under construction to be this.
     */
    ast_container();
    
    /** sets the container under construction to be this.
        Members are not copied.
        @param src source object.
     */
    ast_container(const ast_container &src);

    /** the assignment operator.
        The members are not copied.
        @param src source object.
        @return reference to this.
     */
    ast_container &operator = (const ast_container &src) {
        return *this;
    }

    /** returns the vector of AST members.
        @return the vector of AST members.
     */
    const ast_member_vector &members() const {
        return m_members;
    }

    /** Asks all members to construct themselves from the stack.
        The members are asked to construct themselves in reverse order.
        from a node stack.
        @param b begin position in the source.
        @param e end position in the source.
        @param st stack.
     */
    virtual void construct(const pos &b, const pos &e, ast_stack &st);
    
private:
    ast_member_vector m_members;
    
    friend class ast_member;
}; 


/** Base class for children of ast_container.
 */
class ast_member {
public:
    /** automatically registers itself to the container under construction.
     */
    ast_member() { _init(); }
    
    /** automatically registers itself to the container under construction.
        @param src source object.
     */
    ast_member(const ast_member &src) { _init(); }
    
    /** the assignment operator.
        @param src source object.
        @return reference to this.
     */
    ast_member &operator = (const ast_member &src) {
        return *this;
    }
    
    /** interface for filling the the member from a node stack.
        @param st stack.
     */
    virtual void construct(ast_stack &st) = 0;
    
private:
    //register the AST member to the current container.
    void _init();  
};


/** pointer to an AST object.
    It assumes ownership of the object.
 */
template <class T> class ast_ptr : public ast_member {
public:
    /** the default constructor.
        @param obj object.
     */
    ast_ptr(T *obj = 0) : m_ptr(obj) {
    }
    
    /** the copy constructor.
        It duplicates the underlying object.
        @param src source object.
     */
    ast_ptr(const ast_ptr<T> &src) : 
        m_ptr(src.m_ptr ? new T(*src.m_ptr) : 0)
    {
    }
    
    /** deletes the underlying object.    
     */
    ~ast_ptr() {
        delete m_ptr;
    }
    
    /** copies the given object.
        The old object is deleted.
        @param obj new object.
        @return reference to this.
     */
    ast_ptr<T> &operator = (const T *obj) {
        delete m_ptr;
        m_ptr = obj ? new T(*obj) : 0;
        return *this;
    }
    
    /** copies the underlying object.
        The old object is deleted.
        @param src source object.
        @return reference to this.
     */
    ast_ptr<T> &operator = (const ast_ptr<T> &src) {
        delete m_ptr;
        m_ptr = src.m_ptr ? new T(*src.m_ptr) : 0;
        return *this;
    }
    
    /** gets the underlying ptr value.
        @return the underlying ptr value.
     */
    T *get() const {
        return m_ptr;
    }
    
    /** auto conversion to the underlying object ptr.
        @return the underlying ptr value.        
     */
    operator T *() const {
        return m_ptr;
    }
    
    /** member access.
        @return the underlying ptr value.
     */
    T *operator ->() const {
        assert(m_ptr);
        return m_ptr;
    }

    /** Pops a node from the stack.
        @param st stack.
        @exception std::logic_error thrown if the node is not of the appropriate type.
     */
    virtual void construct(ast_stack &st) {
        assert(!st.empty());
        
        //get a node from the stack
        ast_node *node = st.back();
        
        //check if the node is of type T
        T *obj = dynamic_cast<T *>(node);
        
        //throw an error if there is a logic mistake
        if (!obj) throw std::logic_error("invalid AST node");
        
        //remove the node from the stack
        st.pop_back();
        
        //set the new pointer
        delete m_ptr;
        m_ptr = obj;        
    }
    
private:
    //ptr
    T *m_ptr;
}; 


/** AST function which creates an object of type T 
    and pushes it to the node stack.
 */
template <class T> class ast {
public:
    /** constructor.
        @param r rule to attach the AST function to.
     */
    ast(rule &r) {
        r.set_parse_proc(&_parse_proc);
    }
    
private:
    //parse proc
    static void _parse_proc(const pos &b, const pos &e, void *d) {
        ast_stack *st = reinterpret_cast<ast_stack *>(d);
        T *obj = new T;
        obj->construct(b, e, *st);
        st->push_back(obj);
    }
}; 


/** parses the given input.
    @param i input.
    @param g root rule of grammar.
    @param ws whitespace rule.
    @param el list of errors.
    @param d user data, passed to the parse procedures.
    @return pointer to ast node created, or null if there was an error.
        The return object must be deleted by the caller.
 */
ast_node *parse(input &i, rule &g, rule &ws, error_list &el);


} //namespace parserlib


#endif //AST_HPP