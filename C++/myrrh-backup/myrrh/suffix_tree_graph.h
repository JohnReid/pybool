/**
@file

Copyright John Reid 2007

*/

#ifndef MYRRH_SUFFIX_TREE_GRAPH_H_
#define MYRRH_SUFFIX_TREE_GRAPH_H_

#ifdef _MSC_VER
# pragma once
#endif //_MSC_VER

#include "myrrh/suffix_tree.h"

#include <boost/graph/adjacency_list.hpp>

namespace myrrh {
namespace suffix_tree {

template< 
	typename Node, 
	typename MutableGraph,
	typename EdgeLabelPropertyMap,
	typename EdgeStylePropertyMap
>
struct graph_builder : boost::noncopyable
{
	typedef typename Node::ptr node_ptr;
	typedef typename MutableGraph::vertex_descriptor vertex_descriptor;
	typedef typename MutableGraph::edge_descriptor edge_descriptor;
	typedef std::map< node_ptr, vertex_descriptor > node_vertex_map;

	node_vertex_map _nodes;
	MutableGraph & _g;
	EdgeLabelPropertyMap & _edge_labels;
	EdgeStylePropertyMap * _edge_styles;

	graph_builder(
		MutableGraph & g,
		EdgeLabelPropertyMap & edge_labels,
		EdgeStylePropertyMap * edge_styles = 0 )
		: _g(g)
		, _edge_labels(edge_labels) 
		, _edge_styles(edge_styles) 
	{ }
	vertex_descriptor vertex( node_ptr n )
	{
		using namespace boost;
		BOOST_ASSERT( 0 != n );
		node_vertex_map::iterator v = _nodes.find( n );
		if( _nodes.end() == _nodes.find( n ) ) 
		{
#ifdef MYRRH_DEBUG_SUFFIX_TREE
			//std::cout << "Adding node for " <<n<< "\n";
#endif //MYRRH_DEBUG_SUFFIX_TREE
			tie( v, tuples::ignore ) = _nodes.insert( std::make_pair( n, add_vertex( _g ) ) );
		}
		return v->second;
	}

	void operator()( node_ptr n ) 
	{
		vertex_descriptor v = vertex( n );
		if( 0 != n->_parent )
		{
			using namespace boost;
			vertex_descriptor v_parent = vertex( n->_parent );
			std::pair<edge_descriptor, bool> added = add_edge( v_parent, v, _g );
			put( _edge_labels, added.first, std::string( n->begin(), n->end() ) );
		}
		if( _edge_styles )
		{
			if( 0 != n->_suffix_link )
			{
				vertex_descriptor v_suffix = vertex( n->_suffix_link );
#ifdef MYRRH_DEBUG_SUFFIX_TREE
				//std::cout << "Inserting edge from (" <<v<< "," <<n<< ") to " <<v_suffix<< "," <<n->_suffix_link<< ")\n";
#endif //MYRRH_DEBUG_SUFFIX_TREE
				edge_descriptor e;
				bool inserted;
				tie( e, inserted ) = add_edge( v, v_suffix, _g );
				BOOST_ASSERT( inserted ); //we only add this edge once
				put( *_edge_styles, e, "dashed" );
			}
		}
	}
};

template< 
	typename NodePtrT,
	typename MutableGraph,
	typename EdgeLabelPropertyMap,
	typename EdgeStylePropertyMap
>
void build_graph( 
	NodePtrT root, 
	MutableGraph & g,
	EdgeLabelPropertyMap & edge_labels,
	EdgeStylePropertyMap * edge_styles )
{
	typedef typename NodePtrT::value_type node;
#ifdef MYRRH_DEBUG_SUFFIX_TREE
	validator< typename node::derived >()( root.get(), true );
#endif //MYRRH_DEBUG_SUFFIX_TREE
	typedef graph_builder< 
		node, 
		MutableGraph, 
		EdgeLabelPropertyMap,
		EdgeStylePropertyMap
	> builder_t;
	builder_t builder( g, edge_labels, edge_styles );
	root->traverse< builder_t & >( builder );
}

struct edge_style_t {
	typedef boost::edge_property_tag kind;
};
typedef boost::property< boost::edge_name_t, std::string > EdgeNameProperty;
typedef boost::property< edge_style_t, std::string, EdgeNameProperty > EdgeStyleProperty;
typedef boost::adjacency_list<
	boost::vecS,
	boost::vecS, 
	boost::directedS,
	boost::no_property,
	EdgeStyleProperty
> default_graph_t;


template < class Graph >
struct edge_name_style_writer 
{
	Graph & _g;
	edge_name_style_writer( Graph & g ) : _g( g ) {}
	template< typename Edge >
	void operator()( std::ostream & out, const Edge & e ) const 
	{
		using namespace boost;
		out << "[label=\"" << get( edge_name, _g )[ e ] << "\"]";
		out << "[style=\"" << get( edge_style_t(), _g )[ e ] << "\"]";
	}
};

template< typename NodeT >
void
write_dot_file( typename NodeT::shared_ptr root, const std::string & filename )
{
	using namespace boost;
	default_graph_t g;
	build_graph( root, g, get( edge_name, g ), addressof( get( edge_style_t(), g ) ) );
	write_graphviz( 
		std::ofstream( filename.c_str(), std::ios_base::out ), 
		g, 
		default_writer(),
		edge_name_style_writer< default_graph_t >( g ) );
}


} //namespace suffix_tree
} //namespace myrrh


#endif //MYRRH_SUFFIX_TREE_GRAPH_H_

