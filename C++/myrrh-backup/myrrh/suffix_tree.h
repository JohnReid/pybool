/**
@file

Copyright John Reid 2007

*/

#ifndef MYRRH_SUFFIX_TREE_H_
#define MYRRH_SUFFIX_TREE_H_

#ifdef _MSC_VER
# pragma once
#endif //_MSC_VER

#include "myrrh/defs.h"
#include "myrrh/bit_set.h"

#include <boost/pointee.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/range.hpp>
#include <boost/tuple/tuple.hpp>

#include <set>
#include <deque>
#include <string>
#include <iostream>
#include <iomanip>

namespace myrrh {
namespace suffix_tree {


template< typename IteratorT, typename Derived >
struct node_types
{
	typedef Derived derived;
	typedef derived * ptr;
	typedef ptr suffix_ptr;
	typedef boost::shared_ptr< derived > shared_ptr;
	typedef boost::scoped_ptr< derived > scoped_ptr;
	typedef IteratorT iterator;
	typedef typename boost::pointee< iterator >::type char_t;
	typedef std::basic_string< typename boost::remove_const< char_t >::type > string;

	struct lookup_result
	{
		ptr found;
		iterator end;

		lookup_result( ) : found(0) { }
		lookup_result( ptr found, iterator end ) : found(found), end(end) { }

		operator bool() const { return 0 != found; }
		bool ends_at_node() const { return end == found->end(); }
		string suffix_for() const {
			string result;
			if( found->_parent ) found->_parent->_build_string_for( result );
			result.append( found->begin(), end );
			return result;
		}
	};

	struct label
	{
		typedef typename std::iterator_traits< iterator >::difference_type size_t;
		iterator _begin, _end;
		label( iterator begin, iterator end ) : _begin( begin ), _end( end ) { }
		iterator begin() const { return _begin; }
		iterator end() const { return _end; }
		size_t size() const { return _end - _begin; }
	};

	template< typename Fn >
	struct suffix_visitor
	{
		string _suffix;
		Fn _fn;
		suffix_visitor( Fn fn ) : _fn( fn ) { }
		void enter_node( ptr n ) {
			for( iterator i = n->begin(); n->end() != i; ++i )
			{
				_suffix.push_back( *i );
				_fn( _suffix.begin(), _suffix.end() );
			}
		}
		void leave_node( ptr n ) {
			_suffix.resize( _suffix.size() - n->size() );
		}
	};
};

/**
The IteratorT is an iterator over the string we are building the suffix tree for.
The NodeWithChildren stores the child nodes and implements the lookup, insert and traversal methods for them.
*/
template< typename IteratorT, typename Derived >
struct node 
	: boost::noncopyable
	, node_types< IteratorT, Derived >
{
	ptr _parent;
	label _label;
	suffix_ptr _suffix_link;

	ptr _this() { return static_cast<ptr>(this); }
	const ptr _this() const { return static_cast<const ptr>(this); }

	node( iterator begin, iterator end ) : _parent( 0 ), _label( begin, end ), _suffix_link( 0 ) { }

	void set_label( iterator begin, iterator end ) { _label._begin = begin; _label._end = end; }
	iterator begin() const { return _label.begin(); }
	iterator end() const { return _label.end(); }
	typename label::size_t size() const { return _label.size(); }
	bool is_root() const { return ! _parent; }
	string suffix_for() const {
		string result;
		_build_string_for( result );
		return result;
	}
	void _build_string_for( string & s ) const {
		if( _parent ) _parent->_build_string_for( s );
		s.append( begin(), end() );
	}
};

template< typename NodeT >
struct suffix_printer
{
	void operator()( typename NodeT::string::const_iterator begin, typename NodeT::string::const_iterator end ) const
	{
		std::copy( begin, end, std::ostream_iterator< typename NodeT::char_t >( std::cout ) );
		std::cout << "\n";
	}
};

/**
Creates nodes that are creatable just by the beginning and end of its label.
*/
template< typename NodeT >
struct simple_node_creator 
	: node_types< typename NodeT::iterator, typename NodeT::derived >
{
	ptr new_internal_node( iterator begin, iterator end ) { return new derived( begin, end ); }
	ptr new_leaf_node( iterator begin, iterator end ) { return new derived( begin, end ); }
};

/**
A node that has a bit set to store which sequences it appears in.
*/
template< typename IteratorT, typename Derived >
struct multi_membership
	: node_types< IteratorT, Derived >
{
	myrrh::bit_set<> _bit_set;

	struct node_creator {
		unsigned _num_seqs, _this_seq_idx;
		node_creator( unsigned num_seqs, unsigned this_seq_idx ) : _num_seqs( num_seqs ), _this_seq_idx( this_seq_idx ) { }
		ptr new_internal_node( iterator begin, iterator end ) const { return setup( new derived( begin, end ) ); }
		ptr new_leaf_node( iterator begin, iterator end ) const { return setup( new derived( begin, end ) ); }
		ptr setup( ptr result ) const { result->_bit_set.resize( _num_bits ); result->_bit_set.set( _this_seq_idx ); }
	};
};


/**
A suffix tree node that stores its children as a sibling list
*/
template< typename IteratorT, typename Derived >
struct sibling_list
	: node_types< IteratorT, Derived >
{
	scoped_ptr _children;
	scoped_ptr _sibling;

	ptr _this() { return static_cast<ptr>(this); }
	const ptr _this() const { return static_cast<const ptr>(this); }

	/** Locates a string in children of this node. I.e. it does not take into account the edge arriving in this node. 
	Assumes the string exists so does not check every character, just the first of each child. */
	lookup_result locate( iterator to_lookup_begin, iterator to_lookup_end )
	{
		using namespace boost;

		//return this node if nothing to lookup
		if( to_lookup_begin == to_lookup_end ) return lookup_result( _this(), _this()->end() );

		//iterate through the children
		ptr c = _children.get();
		while( true )
		{
			//we must have a child if we haven't located the string yet
			BOOST_ASSERT( c );

			//did we find the string?
			if( *(c->begin()) == *to_lookup_begin ) 
			{
				//yes - does the string end in the child?
				const int child_size = c->size();
				const int lookup_size = to_lookup_end - to_lookup_begin;
				//yes - so return correct result
				if( child_size > lookup_size ) return lookup_result( c, prior( c->end(), child_size - lookup_size ) );
				//no - carry on looking further down tree
				else return c->locate( next( to_lookup_begin, child_size ), to_lookup_end );
			}
			//no - go to next child
			else c = c->_sibling.get();
		}
	}

	/** 
	Find the (possibly implicit) node that exactly represents the given suffix. 
	*/
	lookup_result lookup( iterator to_lookup_begin, iterator to_lookup_end )
	{
		if( to_lookup_begin == to_lookup_end ) return lookup_result( _this(), _this()->begin() );

		iterator lookup_it = to_lookup_begin;
		iterator node_it = _this()->begin();
		while( true )
		{
			//if we reached the end of what we are looking up we have found it
			if( to_lookup_end == lookup_it ) return lookup_result( _this(), node_it );

			//if we have reached the end of the label then we carry on with the
			//child if we have one otherwise we have not found it
			if( _this()->end() == node_it ) 
			{
				if( _children ) return _children->lookup( lookup_it, to_lookup_end );
				else return lookup_result();
			}

			//if the characters mismatch then we have not found it
			if( *lookup_it != *node_it )
			{
				//try the next sibling if we have one and we are on the first character, otherwise we could not find it
				if( lookup_it == to_lookup_begin && _sibling ) return _sibling->lookup( to_lookup_begin, to_lookup_end );
				else return lookup_result();
			}

			//move along now - nothing to see here
			++lookup_it;
			++node_it;
		}
	}

	/** 
	Find the (possibly implicit) node that represents the longest match to the given suffix. 

	Returns the node in lookup_result and an iterator to the first mismatch character.
	*/
	boost::tuple< lookup_result, iterator >
	lookup_longest( iterator to_lookup_begin, iterator to_lookup_end )
	{
		if( to_lookup_begin == to_lookup_end ) return boost::make_tuple( lookup_result( _this(), _this()->begin() ), to_lookup_begin );

		iterator lookup_it = to_lookup_begin;
		iterator node_it = _this()->begin();
		while( true )
		{
			//if we reached the end of what we are looking up we have found it
			if( to_lookup_end == lookup_it ) return boost::make_tuple( lookup_result( _this(), node_it ), lookup_it );

			//if we have reached the end of the label then we carry on with the
			//child if we have one otherwise we have not found it
			if( _this()->end() == node_it ) 
			{
				if( _children ) return _children->lookup_longest( lookup_it, to_lookup_end );
				else return boost::make_tuple( lookup_result( _this(), _this()->end() ), lookup_it ); //return a partial match
			}

			//if the characters mismatch then we have not found it
			if( *lookup_it != *node_it )
			{
				//try the next sibling if we have one and we are on the first character, otherwise we could not find it
				if( lookup_it == to_lookup_begin && _sibling ) return _sibling->lookup_longest( to_lookup_begin, to_lookup_end );
				else {
					//if we are on the first character then return the end of the parent
					if( lookup_it == to_lookup_begin ) return boost::make_tuple( lookup_result( _this()->_parent, _this()->_parent->end() ), lookup_it );
					else return boost::make_tuple( lookup_result( _this(), node_it ), lookup_it ); //otherwise return where we are at the moment
				}
			}

			//move along now - nothing to see here
			++lookup_it;
			++node_it;
		}
	}

	template< typename NodeCreator >
	ptr insert( iterator insertion_point, NodeCreator node_creator )
	{
		BOOST_ASSERT( 0 != _this()->_parent );
		ptr parent = _this()->_parent;
		
		//remove this from parent
		scoped_ptr _this_scoped;
		parent->remove_child( _this(), _this_scoped );

		//add new node to parent
		ptr new_node( node_creator.new_internal_node( _this()->begin(), insertion_point ) );
		parent->add_child( new_node );

		//reconnect this as child of new inserted node
		new_node->_children.swap( _this_scoped );
		_this()->_parent = new_node;

		_this()->set_label( insertion_point, _this()->end() );
#ifdef MYRRH_DEBUG_SUFFIX_TREE
		validator< derived >()( _this() );
#endif //MYRRH_DEBUG_SUFFIX_TREE
		return new_node;
	}

	void add_child( ptr new_child )
	{
		//cannot add a child that already has a parent
		BOOST_ASSERT( ! new_child->_parent );

		//cannot add a child that already has siblings
		BOOST_ASSERT( ! new_child->_sibling );

		//put at beginning of children sibling list
		new_child->_sibling.swap( _children );
		_children.reset( new_child );
		new_child->_parent = _this();
	}

	void remove_child( ptr child, scoped_ptr & removed )
	{
		//must not have anything passed in the return value argument
		BOOST_ASSERT( ! removed );

		ptr to_remove = _children.get();
		ptr previous_sibling = 0;
		while( child != to_remove )
		{
			previous_sibling = to_remove;
			to_remove = to_remove->_sibling.get();
		}
		if( ! previous_sibling )
			this->_children.swap( to_remove->_sibling );
		else
			previous_sibling->_sibling.swap( to_remove->_sibling );
		to_remove->_parent = 0;
		removed.swap( to_remove->_sibling );
	}

	/** Traverses all the explicit nodes in the tree. */
	template< typename Fn >
	void traverse( Fn fn )
	{
		fn( _this() );
		if( _sibling ) _sibling->traverse< Fn >( fn );
		if( _children ) _children->traverse< Fn >( fn );
	}

	/** Traverses all the implicit nodes in the tree. */
	template< typename Fn >
	void _visit_suffixes( suffix_visitor< Fn > & visitor )
	{
		if( _sibling ) _sibling->_visit_suffixes( visitor );
		visitor.enter_node( _this() );
		if( _children ) _children->_visit_suffixes( visitor );
		visitor.leave_node( _this() );
	}

	/** Traverses all the implicit nodes in the tree. */
	template< typename Fn >
	void visit_suffixes( Fn fn )
	{
		_visit_suffixes( suffix_visitor< Fn >( fn ) );
	}

	bool is_leaf() const { return ! _children; }

};

template< typename IteratorT >
struct sibling_list_node
	: node< IteratorT, sibling_list_node< IteratorT > >
	, sibling_list< IteratorT, sibling_list_node< IteratorT > >
{
	sibling_list_node( iterator begin, iterator end ) : node< IteratorT, sibling_list_node< IteratorT > >( begin, end ) { }
};

template< typename IteratorT >
struct multi_sibling_list_node
	: node< IteratorT, multi_sibling_list_node< IteratorT > >
	, sibling_list< IteratorT, multi_sibling_list_node< IteratorT > >
	, multi_membership< IteratorT, multi_sibling_list_node< IteratorT > >
{
	multi_sibling_list_node( iterator begin, iterator end ) : node< IteratorT, multi_sibling_list_node< IteratorT > >( begin, end ) { }
};

template< typename NodeT >
bool
check_suffix_link( typename NodeT::ptr source, typename NodeT::ptr dest )
{
	using namespace boost;
	typedef typename NodeT::string string;
	const string source_suffix = source->suffix_for();
	const string dest_suffix = dest->suffix_for();
#ifdef MYRRH_DEBUG_SUFFIX_TREE
	std::cout << "Checking \"" <<source_suffix<< "\" -> \"" <<dest_suffix<< "\" is a valid suffix link\n";
#endif //MYRRH_DEBUG_SUFFIX_TREE
	return std::equal( next( source_suffix.begin() ), source_suffix.end(), dest_suffix.begin() );
}

/** Validates suffix tree data structures. */
template< typename Node >
struct validator
{
	typedef typename Node::ptr node_ptr;
	typedef std::set< node_ptr > node_ptr_set;

	node_ptr_set _visited;

	/** Validates that nodes are only visited once in each traversal,
	that parents are assigned correctly and
	that only the root has no parent. */
	struct structure_validator
	{
		node_ptr_set & _visited;
		node_ptr _root;
		structure_validator( node_ptr_set & visited, node_ptr root ) : _visited( visited ), _root( root ) { }
		void operator()( node_ptr n ) 
		{
			if( _visited.find( n ) != _visited.end() ) 
				throw std::logic_error( "Already visited this node - not a tree!" );
			_visited.insert( n );

			node_ptr child = n->_children.get();
			while( child )
			{
				if( child->_parent != n ) 
					throw std::logic_error( "Incorrect parent assignment" );
				child = child->_sibling.get();
			}

			if( n != _root && 0 == n->_parent )
				throw std::logic_error( "Node has no parent but is not root" );

			//if( ! n->is_leaf() && n != _root && ! n->_suffix_link )
				//throw std::logic_error( "Node is non-root internal node but has no suffix link" );
		}
	};

	/** Validates that suffix links point to other nodes in the tree. */
	struct suffix_link_validator
	{
		node_ptr_set & _visited;
		suffix_link_validator( node_ptr_set & visited ) : _visited( visited ) { }
		void operator()( node_ptr n ) 
		{
			if( n->_suffix_link )
				if( _visited.end() == _visited.find( n->_suffix_link ) )
					throw std::logic_error( "Suffix link does not point to node in tree" );
#ifdef MYRRH_DEBUG_SUFFIX_TREE
				//else std::cout << n->_suffix_link << " is a suffix link that is in the tree\n";
#endif //MYRRH_DEBUG_SUFFIX_TREE
		}
	};

	void operator()( node_ptr n, bool is_root = false )
	{
		if( is_root && n->_parent ) throw std::logic_error( "Node is supposed to be root but has parent" );
		n->traverse( structure_validator( _visited, n ) );
		if( is_root ) n->traverse( suffix_link_validator( _visited ) );
	}
};


namespace ukkonen {

template< typename NodeT >
typename NodeT::lookup_result
suffix_link_lookup( 
	typename NodeT::ptr root,
	typename NodeT::lookup_result end_node,
	typename NodeT::iterator begin,
	typename NodeT::iterator end )
{
	using namespace boost;
	typedef typename NodeT::ptr ptr;
	typedef typename NodeT::iterator iterator;
	typedef typename NodeT::lookup_result lookup_result;
	typedef typename NodeT::suffix_ptr suffix_ptr;

#ifdef MYRRH_DEBUG_SUFFIX_TREE
	std::cout << "\nLast end node was: \"" << end_node.suffix_for() << "\"; looking for: \"" << std::string( begin, end ) << "\"\n";
	//std::cout << "Ends at node: " << (end_node.ends_at_node() ? "true\n" : "false\n");
	lookup_result slow_result = root->lookup( begin, end );
#endif //MYRRH_DEBUG_SUFFIX_TREE

	ptr n = end_node.found;
	if( ! n->is_root() ) 
	{
		begin = end;
#ifdef MYRRH_DEBUG_SUFFIX_TREE
		//std::cout << "Not root, now looking for: \"\"\n";
#endif //MYRRH_DEBUG_SUFFIX_TREE
	}

#ifdef MYRRH_DEBUG_SUFFIX_TREE
	if( n->_suffix_link )
		std::cout << "Have suffix link to: \"" <<n->_suffix_link->suffix_for()<< "\"\n";
#endif //MYRRH_DEBUG_SUFFIX_TREE

	//go once up tree if not at root and don't have suffix link
	if( ! n->_suffix_link && ! n->is_root() )
	{
		int chars_to_skip = end_node.end - n->begin();
		n = n->_parent;
		if( n->is_root() ) --chars_to_skip;
		begin = prior( begin, chars_to_skip );
#ifdef MYRRH_DEBUG_SUFFIX_TREE
		std::cout << "Skipping " <<chars_to_skip<< " chars up tree to: \"" << n->suffix_for() << "\"; "
			<< "now looking for \"" << std::string( begin, end ) << "\"\n";
#endif //MYRRH_DEBUG_SUFFIX_TREE
	}

	//if we're not at root traverse the suffix link
	if( ! n->is_root() )
	{
		//if( ! n->_suffix_link )
			//FIXED: fix this - there should be a link here
			//return slow_result;
		BOOST_ASSERT( n->_suffix_link );
		
		n = n->_suffix_link;
#ifdef MYRRH_DEBUG_SUFFIX_TREE
		std::cout << "Followed suffix link to: \"" << n->suffix_for() << "\"\n";
#endif //MYRRH_DEBUG_SUFFIX_TREE
	}

	//lookup what we skipped by moving up earlier
	lookup_result suffix_result = n->locate( begin, end );

#ifdef MYRRH_DEBUG_SUFFIX_TREE
	std::cout << "Located: \"" << suffix_result.suffix_for() << "\"\n";
	BOOST_ASSERT( suffix_result.found == slow_result.found );
	BOOST_ASSERT( suffix_result.end == slow_result.end );
#endif //MYRRH_DEBUG_SUFFIX_TREE

	return suffix_result;
}

template< typename NodeT >
void
make_suffix_link( typename NodeT::ptr source, typename NodeT::ptr dest )
{
#ifdef MYRRH_DEBUG_SUFFIX_TREE
	std::cout << "Suffix linking: \"" << source->suffix_for() << "\" to \"" << dest->suffix_for() << "\"\n";
	if( source->_suffix_link )
		std::cout << "Suffix already linked to: \"" << source->_suffix_link->suffix_for() << "\"\n";
#endif //MYRRH_DEBUG_SUFFIX_TREE
	BOOST_ASSERT( check_suffix_link< NodeT >( source, dest ) );
	//BOOST_ASSERT( ! source->_suffix_link );
	source->_suffix_link = dest;
}


/**
First part of return value is true if rule 3 applies - i.e. we need do no more extensions in this phase
Second part of return value is last inserted internal node
Third part of return value is the last implicit node we found
*/
template< typename NodeT, typename NodeCreator >
boost::tuple< 
	bool, 
	typename NodeT::suffix_ptr,
	typename NodeT::lookup_result
>
do_extension(
	typename NodeT::ptr root,
	typename NodeT::lookup_result active_node,
	typename NodeT::iterator beta_begin,
	typename NodeT::iterator beta_end,
	typename NodeT::iterator end_of_input,
	typename NodeT::iterator phase_end,
	typename NodeT::suffix_ptr suffix_link_source,
	NodeCreator node_creator )
{
	using namespace boost;
	typedef typename NodeT::scoped_ptr scoped_ptr;
	typedef typename NodeT::ptr ptr;
	typedef typename NodeT::iterator iterator;
	typedef typename NodeT::char_t char_t;
	typedef typename NodeT::lookup_result lookup_result;
	typedef typename NodeT::suffix_ptr suffix_ptr;
	typedef boost::tuple< bool, suffix_ptr, lookup_result > result;

#ifdef MYRRH_DEBUG_SUFFIX_TREE
	const string beta( beta_begin, beta_end );
	const char_t test_char = *beta_end;
	std::cout << "extension:  [" << std::setw(10) << beta << "] " << test_char << '\n';
#endif //MYRRH_DEBUG_SUFFIX_TREE

	iterator ext_end = next(beta_end);
	lookup_result path_end = suffix_link_lookup< NodeT >( root, active_node, beta_begin, beta_end );
	BOOST_ASSERT( path_end ); //we must have found it
	const bool ends_at_node = path_end.ends_at_node();
	bool extension_rule_3 = false;		//this will be the first part of the return value
	ptr new_internal_node = 0;			//the second part of the return value
	if( ends_at_node && path_end.found->is_leaf() && root != path_end.found ) //case 1
	{
		//int i = 0;
		//nothing to do now all leaves are set to end at end of input - once a leaf, always a leaf
		//path_end.found->set_label( 
		//	beta_begin,
		//	ext_end );
	}
	else
	{
		//now we must consider if our path ends at a node or not
		const bool extension_already_in_tree =
			ends_at_node 
				? path_end.found->lookup( beta_end, ext_end ) //if it ends at a node we need to lookup in the next node
				: *(path_end.end) == *(beta_end); //otherwise we can just test the next character

		if( ! extension_already_in_tree ) //case 2
		{
			//if we did not end at a node we must split the edge and insert a new node
			if( ! ends_at_node )
			{
				ptr new_node = path_end.found->insert( path_end.end, node_creator );
#ifdef MYRRH_DEBUG_SUFFIX_TREE
				std::cout << "Inserted new internal node: \"" << new_node->suffix_for() << "\"\n";
#endif //MYRRH_DEBUG_SUFFIX_TREE
				path_end = lookup_result( new_node, new_node->end() ); 
			}
#ifdef MYRRH_DEBUG_SUFFIX_TREE
			std::cout << "inserting: \"" << string( beta_end, end_of_input ) << "\"\n";
#endif //MYRRH_DEBUG_SUFFIX_TREE
			path_end.found->add_child( node_creator.new_leaf_node( beta_end, end_of_input ) );
			new_internal_node = path_end.found;
		}
		else //case 3
		{
			//we might think the extension is already in the tree as we have set up our leaf nodes to extend
			//to the end of the input. let us check that here
			extension_rule_3 = path_end.end < beta_end;
		}
	}
	if( suffix_link_source ) make_suffix_link< NodeT >( suffix_link_source, path_end.found );
	return result( extension_rule_3, new_internal_node, path_end );
}


template< typename NodeT, typename NodeCreator >
typename NodeT::iterator
do_phase( 
	typename NodeT::ptr root,
	typename NodeT::iterator phase_begin,
	typename NodeT::iterator phase_end,
	typename NodeT::iterator end_of_input,
	NodeCreator node_creator )
{
	using namespace boost;
	typedef typename NodeT::scoped_ptr scoped_ptr;
	typedef typename NodeT::suffix_ptr suffix_ptr;
	typedef typename NodeT::lookup_result lookup_result;
	typedef typename NodeT::iterator iterator;
	typedef typename NodeT::char_t char_t;

#ifdef MYRRH_DEBUG_SUFFIX_TREE
	const string phase( phase_begin, next( phase_end ) );
	std::cout << "phase:     \"" << phase << "\"\n";
#endif //MYRRH_DEBUG_SUFFIX_TREE

	//do each extension
	iterator ext_begin = phase_begin;
	suffix_ptr suffix_link_parent( 0 );
	lookup_result end_point( root, root->end() );
	while( true )
	{
		bool end_of_phase;
		tie( end_of_phase, suffix_link_parent, end_point ) = 
			do_extension< NodeT >( 
				root, 
				end_point, 
				ext_begin, 
				phase_end, 
				end_of_input,
				phase_end,
				suffix_link_parent, 
				node_creator );
		if( end_of_phase || phase_end == ext_begin ) return ext_begin;
		++ext_begin;
	}

}

template< typename NodeT, typename NodeCreator >
void
update_tree(
	typename NodeT::ptr root,
	typename NodeT::iterator begin,
	typename NodeT::iterator end,
	typename NodeT::iterator phase_end,
	NodeCreator node_creator )
{
	using namespace boost;
	typedef typename NodeT::scoped_ptr scoped_ptr;
	typedef typename NodeT::suffix_ptr suffix_ptr;
	typedef typename NodeT::iterator iterator;
	typedef typename NodeT::char_t char_t;
	typedef typename NodeT::string string;

#ifdef MYRRH_DEBUG_SUFFIX_TREE
	const string input( begin, end );
	std::cout << "build tree: " << input << '\n';
#endif //MYRRH_DEBUG_SUFFIX_TREE

	if( begin == end ) return;

	//do each phase
	while( true )
	{
		begin = do_phase< NodeT >( root, begin, phase_end, end, node_creator );
		++phase_end;
#ifdef MYRRH_DEBUG_SUFFIX_TREE
		//static int _i = 0;
		//write_dot_file< NodeT >( root, MYRRH_MAKE_STRING( "graphs/partial_" <<++_i<< ".dot" ) );
#endif //MYRRH_DEBUG_SUFFIX_TREE
		if( end == phase_end ) break;
	}
#ifdef MYRRH_DEBUG_SUFFIX_TREE
	validator< typename NodeT::derived >()( root, true );
#endif //MYRRH_DEBUG_SUFFIX_TREE
}

template< typename Iterator >
typename sibling_list_node< Iterator >::shared_ptr
new_tree(
	Iterator begin,
	Iterator end )
{
	typedef sibling_list_node< Iterator > suffix_tree_node;
	typedef suffix_tree_node::shared_ptr shared_ptr;
	shared_ptr root( new suffix_tree_node( end, end ) );
	update_tree< suffix_tree_node >( root.get(), begin, end, begin, simple_node_creator< suffix_tree_node >() );
	return root;
}

/** Adds new string to suffix tree built on different string. */
template< typename NodeT, typename NodeCreator >
void
incremental_update(
	typename NodeT::ptr root,
	typename NodeT::iterator begin,
	typename NodeT::iterator end,
	NodeCreator node_creator )
{
	using namespace boost;
	typedef typename NodeT::scoped_ptr scoped_ptr;
	typedef typename NodeT::suffix_ptr suffix_ptr;
	typedef typename NodeT::iterator iterator;
	typedef typename NodeT::lookup_result lookup_result;
	typedef typename NodeT::char_t char_t;
	typedef typename NodeT::string string;

	//check whether anything to do
	if( begin == end ) return;

	lookup_result partial;
	iterator partial_match;
	tie( partial, partial_match ) = root->lookup_longest( begin, end );
//#ifdef MYRRH_DEBUG_SUFFIX_TREE
	std::cout << "Have found match for \"" <<string(begin, partial_match)<< "\" in update string \"" <<string(begin, end)<< "\"\n";
	std::cout << "Updating tree with \"" <<string(partial_match, end)<< "\"\n";
//#endif //MYRRH_DEBUG_SUFFIX_TREE
	update_tree< NodeT >( root, begin, end, partial_match, node_creator );
}


} //namespace ukkonen






} //namespace suffix_tree
} //namespace myrrh


#endif //MYRRH_SUFFIX_TREE_H_

