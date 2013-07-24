/**
 * 456789------------------------------------------------------------------------------------------------------------120
 *
 * @brief ...
 * @file dim1algebra.hpp
 * 
 * This file is created at Almende B.V. and Distributed Organisms B.V. It is open-source software and belongs to a
 * larger suite of software that is meant for research on self-organization principles and multi-agent systems where
 * learning algorithms are an important aspect.
 *
 * This software is published under the GNU Lesser General Public license (LGPL).
 *
 * It is not possible to add usage restrictions to an open-source license. Nevertheless, we personally strongly object
 * against this software being used for military purposes, factory farming, animal experimentation, and "Universal
 * Declaration of Human Rights" violations.
 *
 * Copyright (c) 2013 Anne C. van Rossum <anne@almende.org>
 *
 * @author    Anne C. van Rossum
 * @date      Jul 24, 2013
 * @project   Replicator 
 * @company   Almende B.V.
 * @company   Distributed Organisms B.V.
 * @case      Sensor fusion
 */

#ifndef DIM1ALGEBRA_HPP_
#define DIM1ALGEBRA_HPP_

#define _GLIBCXX_CONCEPT_CHECKS

// General files
#include <cassert>
#include <algorithm>
#include <functional>
#include <numeric>
#include <iostream>
#include <cmath>
#include <iterator>

/**
 * Some general information for folks that are unaware of the standard STL functions... There are many methods
 * available that should be enough to get you started. Look at <algorithm> and <numeric> for specific implementations.
 * Here you see some examples on use:
 *
 * This multiplies all elements in a vector by a scalar "10":
 *   std::transform(vec.begin(), vec.end(), vec.begin(), std::bind1st(std::multiplies<T>(),10));
 *
 * Or point-wise multiplication of two vectors (same size):
 *   std::transform(v1.begin(), v1.end(), v2.begin(), result.begin(), std::multiplies<T>());
 * It is important to note that a result vector should have allocated enough entries beforehand. This will not be done
 * by the STL methods themselves. It is also easy to see that it is impossible for the function to check if v2 is
 * actually of the same size (or larger) than v1.
 */

/***********************************************************************************************************************
 * Helper functions for 1 dimensional algebra functions for standard containers.
 **********************************************************************************************************************/

namespace dobots {

/**
 * For an explanation of the different metrics, see the "distance" function below. This distance function does really
 * calculate a distance between two containers, say two vectors, and is not the std::distance function that just returns
 * the distance between elements with respect to a given iterator.
 *
 * Only metrics are defined (for now) that do not require additional information. For example, the Mahalanobis distance
 * requires the correlations between the variables as input (either by including standard deviations, or the covariance
 * matrix in the general case).
 */
enum DistanceMetric {
	DM_EUCLIDEAN,
	DM_DOTPRODUCT,
	DM_BHATTACHARYYA,
	DM_HELLINGER,
	DM_MANHATTAN,
	DM_CHEBYSHEV,
	DM_BHATTACHARYYA_COEFFICIENT,
	DM_SQUARED_HELLINGER,
	DM_TYPES
};

enum SetDistanceMetric {
	SDM_INFIMIM,
	SDM_SUPREMUM,
	SDM_HAUSDORFF,
	SDM_SUPINF,
	SDM_TYPES
};

enum Norm {
	N_EUCLIDEAN,
	N_TAXICAB,
	N_MAXIMUM,
	N_TYPES
};

enum Mean {
	M_ARITHMETIC,
	M_GEOMETRIC,
	M_HARMONIC,
	M_QUADRATIC,
	M_TYPES
};

/**
 * Somehow "std::max" is only given as a normal template function and not as a derivation of a function object derived
 * from binary_function. There does not seem to be a reason for that.
 * Background info: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2001/n1293.html
 * Hence, we need this helper function to be able to use max() as a functor.
 */
template<typename _Tp>
struct max : public std::binary_function<_Tp, _Tp, _Tp> {
	_Tp operator()(const _Tp& __x, const _Tp& __y) const
	{ return std::max(__x, __y); }
};

/***********************************************************************************************************************
 * Distances and pseudo-distances
 **********************************************************************************************************************/

/**
 * The p=2 norm, element-wise squaring the difference. This norm is also called the Euclidean norm and is the most known
 * of all.
 */
template<typename T>
T euclidean(T x, T y) {
	return (x-y)*(x-y);
}

/**
 * The p=1 norm, can be used for the Manhattan distance (but also the Chebyshev distance). This norm is a lot "less
 * smooth" than the Euclidean norm. If used for example for regression outliers do have a much higher weight compared
 * to the Euclidean norm (compared to the inliers).
 */
template<typename T>
T taxicab(const T & x, const T & y) {
	return std::abs(x-y);
}

/**
 * See: http://en.wikipedia.org/wiki/Bhattacharyya_distance
 * The Battacharyya distance is related to the Battacharyya coefficient (which measures the overlap between two
 * statistical samples). It is the square root of the product of the entities involved and hence even less robust than
 * the taxicab norm for regression.
 */
template<typename T>
T battacharyya(T x, T y) {
	return std::sqrt(x*y);
}

/**
 * See: http://en.wikipedia.org/wiki/Hellinger_distance
 * Related to the Battacharyya coefficient. It takes the square root of both individual entities, calculates the
 * difference and squares that.
 */
template<typename T>
T hellinger(T x, T y) {
	T tmp = std::sqrt(x)-std::sqrt(y);
	return tmp*tmp;
}

/**
 * The hyperbolic distance is the absolute difference between the log of two variables.
 */
template<typename T>
T hyperbolic(const T & x, const T & y) {
	return std::abs(std::log(x) - std::log(y));
}

/***********************************************************************************************************************
 * Square, inverse, etc. of individual elements
 **********************************************************************************************************************/

/**
 * Template function for the square of a variable. In the std library only the square root is defined (as std::sqrt).
 */
template<typename T>
T square(T x) { return x * x; }

/**
 * Template function for 1/x.
 */
template<typename T>
T inverse(T x) { return T(1)/x; }

/**
 * Template function for absolute does not exist for every platform. Hence, let's just use std::abs() without template
 * parameters.
 */
template<typename T>
T absolute(T x) { return std::abs(x); }

/***********************************************************************************************************************
 * Vector multiplication with a scalar
 **********************************************************************************************************************/

/**
 * Create a template function which moves container x from or towards y with a learning rate "mu". A positive mu will
 * move "x" away, while a negative mu will move "x" towards "y". This is used in for example incremental learning
 * vector quantization.
 */
template<typename T>
class op_adjust: std::binary_function<T,T,T> {
	T mu_;
public:
	op_adjust(T mu): mu_(mu) {}
	T operator()(T x, T y) const {
		//fancy: std::multiplies<T>( std::minus<T>(x,y), mu_);
		T result = x + (x-y)*mu_;
		return result;
	}
};

/**
 *  @brief Accumulate values in a range with one operation for the accumulation and one operation that has to be
 *  performed on the to be accumulated element.
 *
 *  Accumulates the values in the range [first,last) using the function object @a binary_op. The initial value is
 *  @a init. The values are processed in order.
 *
 *  @param first           Start of range.
 *  @param last            End of range.
 *  @param init            Starting value to add other values to.
 *  @param binary_op       Function object to accumulate with.
 *  @param unary_op        Function object on element before accumulation.
 *  @return                The final sum.
 */
template<typename InputIterator, typename T, typename BinaryOperation, typename UnaryOperation>
inline T accumulate(InputIterator first, InputIterator last, T init,
		BinaryOperation binary_op, UnaryOperation unary_op)
{
	// concept requirements
	__glibcxx_function_requires(_InputIteratorConcept<InputIterator>);
	__glibcxx_requires_valid_range(first, last);

	for (; first != last; ++first)
		init = binary_op(init, unary_op(*first));
	return init;
}

/**
 *  @brief  Return the maximum element in a range given one additional operation that is performed on it beforehand.
 *  For example if @ a unary_op is std::abs<T> it returns the location of the element with the maximum absolute value.
 *  @ingroup sorting_algorithms
 *  @param  first  			start of range
 *  @param  last   			end of range
 *  @param  unary_op			unary operation
 *  @return  Iterator referencing the first instance of the largest value.
 */
template<typename ForwardIterator, typename UnaryOperation>
ForwardIterator
max_element(ForwardIterator first, ForwardIterator last, UnaryOperation unary_op)
{
	// concept requirements
	__glibcxx_function_requires(_ForwardIteratorConcept<ForwardIterator>);
//		__glibcxx_function_requires(LessThanComparableConcept<
//				typename iterator_traits<ForwardIterator>::value_type>);
	__glibcxx_requires_valid_range(first, last);

	if (first == last)
		return first;
	ForwardIterator result = first;
	while (++first != last)
		if (unary_op(*result) < unary_op(*first))
			result = first;
	return result;
}


/**
 * This function tells something about the "size" or "length" of a container, mathematically called "norm".
 * There are currently several norms implemented:
 *   N_EUCLIDEAN:			return sqrt (sum_i { (x_i)^2 } ) (L2 norm)
 *   N_TAXICAB:				return sum_i { abs(x_i) } (L1 norm)
 *   N_MAXIMUM				return max_i (x_i) (maximum norm)
 * @param first				start of the container
 * @param last				end of the container
 * @param norm				a certain type of norm, e.g. N_EUCLIDEAN
 * @return				the norm
 */
template<typename T, typename InputIterator>
T norm(InputIterator first, InputIterator last, Norm norm) {
	__glibcxx_function_requires(_InputIteratorConcept<InputIterator>);
	__glibcxx_requires_valid_range(first, last);

	switch (norm) {
	case N_EUCLIDEAN:
		return std::sqrt(accumulate(first, last, T(0), std::plus<T>(), square<T> ) );
	case N_TAXICAB:
		return accumulate(first, last, T(0), std::plus<T>(), absolute<T>);
	case N_MAXIMUM:
		if (std::distance(first,last) == 0) return T(0);
		return *max_element(first, last, absolute<T>);
	default:
		std::cerr << "Unknown norm" << std::endl;
		return T(-1);
	}
}

/**
 * This function tells something about the static ensemble behaviour in the form of a "mean". There are currently
 * several "means" implemented:
 *   M_ARITHMETIC: return 1/n (sum_i { (x_i) } )
 *   M_GEOMETRIC:  return exp ( 1/n (sum_i { log(x_i) } ) )
 *   M_HARMONIC:   return max_i (x_i) (maximum norm)
 * @param first              Start of the container
 * @param last               End of the container
 * @param mean               A certain type of "mean", e.g. M_GEOMETRIC
 * @return                   The value of the mean
 */
template<typename T, typename InputIterator>
T mean(InputIterator first, InputIterator last, Norm norm) {
	__glibcxx_function_requires(_InputIteratorConcept<InputIterator>);
	__glibcxx_requires_valid_range(first, last);
	typedef typename std::iterator_traits<InputIterator>::difference_type DistanceType1;

	DistanceType1 dist = std::distance(first, last);
	if (!dist) return T(0);

	switch (norm) {
	case M_ARITHMETIC:
		return 1/T(dist)*std::accumulate(first, last, T(0));
	case M_GEOMETRIC:
		return std::exp(1/T(dist)*accumulate(first, last, T(0), std::plus<T>(), std::log<T>));
	case M_HARMONIC:
		return T(dist)/accumulate(first, last, T(0), std::plus<T>(), inverse<T>);
	default:
		std::cerr << "Unknown norm" << std::endl;
		return T(-1);
	}
}

/**
 * Incremental adjustment of a standard container towards a reference container.
 *   d = d + mu ( ref - d)
 * So:
 *   delta_d = mu ( ref - d)
 * If "ref" is smaller than "d", "delta_d" will be negative: it will make "d" smaller.
 * Note that this function does not make use of the different distance metrics that can be defined.
 * It will use std::minus (normal -sign) to judge the distance over all elements of the container
 * and adjust them in the same fashion by the multiplication factor mu.
 * If "mu" is 1, it will set the "tomove" container equal to the "reference" container.
 * @param tomove			the container to-be-moved
 * @param reference			the reference container towards the movements happens, the "attractor"
 * @param mu				the step size (0 < mu <= 1)
 */
template<typename T, typename InputIterator1, typename InputIterator2>
void increaseDistance(InputIterator1 tomove_first, InputIterator1 tomove_last, InputIterator2 reference_first, T mu) {
	__glibcxx_function_requires(_InputIteratorConcept<InputIterator1>);
	__glibcxx_function_requires(_InputIteratorConcept<InputIterator2>);
	__glibcxx_requires_valid_range(first1, last1);
	assert (mu > T(0)); assert (mu <= T(1));
	std::transform(tomove_first, tomove_last, reference_first, tomove_first, op_adjust<T>(mu));
}

/**
 * Incremental adjustment of a container back from a reference container.
 *   d = d - mu ( ref - d)
 * @param tomove			the container to-be-moved
 * @param reference			the reference container that functions as a "repeller"
 * @param mu				the step size (0 < mu <= 1)
 * @return				void
 */
template<typename T, typename InputIterator1, typename InputIterator2>
void decreaseDistance(InputIterator1 tomove_first, InputIterator1 tomove_last, InputIterator2 reference_first, T mu) {
	__glibcxx_function_requires(_InputIteratorConcept<InputIterator1>);
	__glibcxx_function_requires(_InputIteratorConcept<InputIterator2>);
	__glibcxx_requires_valid_range(first1, last1);
	assert (mu > T(0)); assert (mu <= T(1));
	std::transform(tomove_first, tomove_last, reference_first, tomove_first, op_adjust<T>(-mu));
}

/**
 * This function tells something about the "distance" between containers, in other words the similarity or
 * dissimilarity. There are currently several metrics implemented:
 *   DM_DOTPRODUCT:			return sum_i { x_i*y_i }
 *   DM_EUCLIDEAN:			return sqrt (sum_i { (x_i-y_i)^2 } )
 *   DM_BHATTACHARYYA:			return -ln (sum_i { sqrt (x_i*y_i) } )
 *   DM_HELLINGER:			return sqrt (sum_i { (sqrt(x_i)-sqrt(y_i))^2 } ) * 1/sqrt(2)
 *   DM_CHEBYSHEV:			return max_i abs(x_i-y_i)
 *   DM_MANHATTAN:			return sum_i { abs(x_i-y_i) }
 * And there are some other measures that can be used as metrics. Such as the Bhattacharyya coefficient
 * and the squared Hellinger distance.
 * It is assumed that the containers are of equal size.
 * @param first1			start of the first container
 * @param last1				end of the first container
 * @param first2			start of the second container
 * @param last2				end of the second container
 * @param metric			a certain distance metric
 * @return				the distance between the two containers
 */
template<typename T, typename InputIterator1, typename InputIterator2>
T distance(InputIterator1 first1, InputIterator1 last1, InputIterator2 first2, InputIterator2 last2, DistanceMetric metric) {
	__glibcxx_function_requires(_InputIteratorConcept<InputIterator1>);
	__glibcxx_function_requires(_InputIteratorConcept<InputIterator2>);
	__glibcxx_requires_valid_range(first1, last1);
	__glibcxx_requires_valid_range(first2, last2);
	if (std::distance(first2,last2) != std::distance(first1,last1)) {
		std::cerr << "Container size unequal: " << std::distance(first1,last1) << " vs " << std::distance(first2,last2)
		<< std::endl;
		assert (std::distance(first2,last2) == std::distance(first1,last1));
	}
	switch (metric) {
	case DM_DOTPRODUCT:
		return std::inner_product(first1, last1, first2, T(0));
	case DM_EUCLIDEAN:
		return std::sqrt(std::inner_product(first1, last1, first2, T(0), std::plus<T>(), euclidean<T>));
	case DM_BHATTACHARYYA:
		return -std::log(std::inner_product(first1, last1, first2, T(0), std::plus<T>(), battacharyya<T>));
	case DM_HELLINGER:
		return (std::sqrt(std::inner_product(first1, last1, first2, T(0), std::plus<T>(), hellinger<T>))) /
				std::sqrt(2);
	case DM_CHEBYSHEV:
		return std::inner_product(first1, last1, first2, T(0), max<T>(), taxicab<T>);
	case DM_MANHATTAN:
		return std::inner_product(first1, last1, first2, T(0), std::plus<T>(), taxicab<T>);
	case DM_BHATTACHARYYA_COEFFICIENT:
		return std::inner_product(first1, last1, first2, T(0), std::plus<T>(), battacharyya<T>);
	case DM_SQUARED_HELLINGER:
		//return std::inner_product(first1, last1, first2, T(0), std::plus<T>(), hellinger<T>) * T(1)/T(2);
		// same result when using battacharyya instead of hellinger, and faster to calculate:
		return std::sqrt(T(1) - std::inner_product(first1, last1, first2, T(0), std::plus<T>(), battacharyya<T>));
	default:
		std::cerr << "Unknown distance metric" << std::endl;
		return T(-1);
	}
}

/***********************************************************************************************************************
 * Point convenience helpers
 **********************************************************************************************************************/

/**
 * Provide a similar template function, but now with containers instead of iterators. Be careful that now the
 * typename Point is not checked for having actually "begin() and end()" operators.
 */
template<typename T, typename Point>
T distance_impl(Point point1, Point point2, DistanceMetric metric) {
	return distance(point1.begin(), point1.end(), point2.begin(), point2.end(), metric);
}


/**
 * A wrapper struct for "distance" to be used as function object (it is a binary function) to calculate
 * distances between sets. It is a comparison functor. To do the comparison the distance between a
 * reference point, previously given, and each of the argument is calculated. If the first argument is
 * smaller, the function returns true.
 * @template_param		Point should be a pointer to a container, e.g. std::vector<double>*
 * @template_param		Iterator should be an iterator over the same type of container
 * @template_param		Value should be the value of the elements in the container
 * @param	 		point_metric to be used, e.g. Euclidean
 * @param			first_ref is reference to point
 * @param			last_ref is reference to point
 * @return			true if x is closer to the reference point than y
 * This struct requires the function of above.
 */
template<typename Point, typename PointIterator, typename PointValueType>
struct comp_point_distance: public std::binary_function<Point, Point, bool> {
	comp_point_distance(DistanceMetric point_metric, PointIterator first_ref, PointIterator last_ref):
		point_metric(point_metric), first_ref(first_ref), last_ref(last_ref) {
	}
	bool operator()(Point x, Point y) {
		return distance<PointValueType, PointIterator, PointIterator>(x->begin(), x->end(), first_ref, last_ref, point_metric) <
				distance<PointValueType, PointIterator, PointIterator>(y->begin(), y->end(), first_ref, last_ref, point_metric);
	}
	DistanceMetric point_metric;
	PointIterator first_ref;
	PointIterator last_ref;
};

/*
 * A function calculating the distance of a point to a set.
 * 	SDM_INFIMIM		the minimum distance to this point, for Euclidean/Manhattan in 1D example, d(1,[3,6]) = 2 and d(7,[3,6]) = 1.
 * TODO: make sure that the values of the iterator over the set correspond with the container over which the second iterator
 * runs.
 */
template<typename T, typename SetIterator, typename PointIterator>
T distance_to_point(SetIterator first_set, SetIterator last_set, PointIterator first_point, PointIterator last_point,
		SetDistanceMetric set_metric, DistanceMetric point_metric) {
	__glibcxx_function_requires(_InputIteratorConcept<SetIterator>);
	__glibcxx_function_requires(_InputIteratorConcept<PointIterator>);
	__glibcxx_requires_valid_range(first1, last1);
	__glibcxx_requires_valid_range(first2, last2);
	typedef typename std::iterator_traits<SetIterator>::value_type PointType; // e.g. std::vector<double>*
	typedef typename std::iterator_traits<PointIterator>::value_type PointValueType; // e.g. double

	T result = T(-1);
	PointType tmp;
	switch(set_metric) {
	case SDM_INFIMIM: // the smallest distance between the point and any point in the set
		tmp = *std::min_element(first_set, last_set, comp_point_distance<PointType, PointIterator, PointValueType>(
				point_metric, first_point, last_point));
		return distance<PointValueType, PointIterator, PointIterator>(tmp->begin(), tmp->end(), first_point, last_point, point_metric);
	case SDM_SUPREMUM: // the largest distance between the point and any point in the set
		tmp = *std::max_element(first_set, last_set, comp_point_distance<PointType, PointIterator, PointValueType>(
				point_metric, first_point, last_point));
		return distance<PointValueType, PointIterator, PointIterator>(tmp->begin(), tmp->end(), first_point, last_point, point_metric);
	default:
		std::cerr << "Not yet implemented" << std::endl;
		break;
	}

	return result;
}

/**
 * Same function as above, but using iterators implicitly. Not safe.
 */
template<typename T, typename Point, typename SetContainer>
T distance_impl(SetContainer set, Point point, SetDistanceMetric set_metric, DistanceMetric point_metric) {
	return distance_to_point(set.begin(), set.end(), point.begin(), point.end(), set_metric, point_metric);
}

/**
 * @template_param		Point should be something like std::vector<float> *
 * @template_param		SetIterator should be an iterator over a set
 * @template_param		PointIterator should be an iterator over the point container
 * @template_param		Value should be the type of the values in the point container, e.g. float
 * @return			true if point is closer to
 */
template<typename Point, typename SetIterator, typename PointIterator, typename Value>
struct comp_point_set_distance: public std::binary_function<Point, Point, bool> {
	comp_point_set_distance(SetDistanceMetric set_metric, DistanceMetric point_metric, SetIterator first_set,
			SetIterator last_set):
				set_metric(set_metric), point_metric(point_metric), first_set(first_set), last_set(last_set) {
	}

	bool operator()(const Point & x, const Point & y) const {
		return distance_to_point<Value, SetIterator, PointIterator>(first_set, last_set, x->begin(), x->end(), set_metric, point_metric) <
				distance_to_point<Value, SetIterator, PointIterator>(first_set, last_set, y->begin(), y->end(), set_metric, point_metric);
	}
	SetDistanceMetric set_metric;
	DistanceMetric point_metric;
	SetIterator first_set;
	SetIterator last_set;
};


/**
 * Different metrics that exist between sets of points.
 *   SDM_HAUSDORFF 		longest distance you can be forced to travel by an adversary who chooses a point in one of the two sets,
 *   				from where you then must travel to the other set (wikipedia)
 *   SDM_SUPINF			calculates the smallest distance between a point and any other, then picks the point that is most remote
 *   				of the others (Hausdorff is then just doing this also in the opposite direction in case distances are not
 *   				symmetric)
 * For now only Hausdorff is implemented and the supremum-infimim operator. An example of the Hausdorff metric when the point metric
 * is   d([1,3,6,7], [3,6]) = 2, but d([3,6], [1,3,6,7]) = 0.
 * @param set0			a set of data points (each data point can be a vector or list or matrix)
 * @param set1			another set of data points
 * @param set_metric		the metric to be used between the two sets
 * @param metric		the metric to be used to define distances between points
 *
 * Caution! The SetIterator and the PointIterator do not correspond to the first set of two iterators, respectively the last set of two
 * iterators. All of these iterators should be of the same type SetIterator. However, they should be decomposable into PointIterators.
 * In other words, the set entities should have the PointIterator as valid iterator defined over each of their elements. This definitely
 * requires you to define the template variables (because they cannot be retrieved from the arguments).
 */
template<typename T, typename SetIterator, typename PointIterator>
T distance_to_set(SetIterator first1, SetIterator last1, SetIterator first2, SetIterator last2,
		SetDistanceMetric set_metric, DistanceMetric point_metric) {
	__glibcxx_function_requires(_InputIteratorConcept<SetIterator>);
	__glibcxx_function_requires(_InputIteratorConcept<PointIterator>);
	__glibcxx_requires_valid_range(first1, last1);
	__glibcxx_requires_valid_range(first2, last2);
	typedef typename std::iterator_traits<SetIterator>::value_type PointType; // e.g. std::vector<double>*
	typedef typename std::iterator_traits<PointIterator>::value_type PointValueType; // e.g. double

	SetDistanceMetric overwrite_set_metric;
	PointType tmp;
	switch(set_metric) {
	case SDM_HAUSDORFF: {
		T dist_xy = distance_to_set<T,SetIterator,PointIterator>(first1, last1, first2, last2, SDM_SUPINF, point_metric);
		T dist_yx = distance_to_set<T,SetIterator,PointIterator>(first2, last2, first1, last1, SDM_SUPINF, point_metric);
		return std::max(dist_xy, dist_yx);
	}
	case SDM_SUPINF:
		overwrite_set_metric = SDM_INFIMIM;
		tmp = *std::max_element(first1, last1,
				comp_point_set_distance<PointType, SetIterator, PointIterator, PointValueType>(
						overwrite_set_metric, point_metric, first2, last2));
		return distance_to_point<PointValueType, SetIterator, PointIterator>(
				first2, last2, tmp->begin(), tmp->end(), overwrite_set_metric, point_metric);
		break;
	default:
		std::cerr << "Not yet implemented" << std::endl;
		break;
	}
}

/***********************************************************************************************************************
 * Integrals, rotations, and convolutions
 **********************************************************************************************************************/

/**
 * Generate a "0 value" (depends on the class T what that constitutes).
 */
template <typename T>
T zero_func() { return T(0); }

/**
 * Generate a series of "0 values" (depends on the class T what that constitutes).
 */
template<typename InputIterator1>
void clean(InputIterator1 first1, InputIterator1 last1) {
	typedef typename std::iterator_traits<InputIterator1>::value_type ValueType1;
	std::generate_n(first1, last1 - first1, zero_func<ValueType1>);
}

/**
 * Calculate the integral of a function given a kernel in the discrete case. It multiplies the individual elements from
 * both input vectors and writes the accumulated sum of the product to the result. With x[i]=i, y[i]=i, the result
 * is: r[0]=0, r[1]=0+1*1=1, r[2]=1+2*2=5, r[3]=5+3*3=14, r[4]=14+4*4=30, etc.
 *
 * Functions like remove_copy which return a series of values in the STL C++ library, require the user to allocate the
 * proper number of items beforehand. This seems to me a pain for functions where this is not known beforehand. However,
 * to be consistent with STL, you will need to have the output iterator pointed to a container that is large enough.
 */
template<typename InputIterator1, typename InputIterator2, typename OutputIterator>
OutputIterator
integral(InputIterator1 first1, InputIterator1 last1, InputIterator2 first2,
		OutputIterator result) {

	typedef typename std::iterator_traits<InputIterator1>::value_type ValueType1;

	// concept requirements
	__glibcxx_function_requires(_InputIteratorConcept<InputIterator1>);
	__glibcxx_function_requires(_InputIteratorConcept<InputIterator2>);
	__glibcxx_function_requires(_OutputIteratorConcept<OutputIterator, ValueType1>);
	__glibcxx_requires_valid_range(first1,last1);

	if (first1 == last1)
		return result;
	ValueType1 value = *first1 * *first2;
	*result = value;
	while (++first1 != last1)
	{
		value = value + *first1 * *++first2;
		*++result = value;
	}
	return ++result;
}

/**
 * As the integral above, but the user can define how to combine the input elements (by e.g. a binary operator different
 * from element-wise multiplication) and how to aggregate the result (by e.g. a binary operator different from
 * addition).
 */
template<typename InputIterator1, typename InputIterator2, typename OutputIterator, typename BinaryOperation1,
typename BinaryOperation2>
OutputIterator
integral(InputIterator1 first1, InputIterator1 last1, InputIterator2 first2,
		OutputIterator result, BinaryOperation1 binary_op1, BinaryOperation2 binary_op2) {

	typedef typename std::iterator_traits<InputIterator1>::value_type ValueType1;

	// concept requirements
	__glibcxx_function_requires(_InputIteratorConcept<InputIterator1>);
	__glibcxx_function_requires(_InputIteratorConcept<InputIterator2>);
	__glibcxx_function_requires(_OutputIteratorConcept<OutputIterator, ValueType1>);
	__glibcxx_requires_valid_range(first1,last1);

	if (first1 == last1)
		return result;
	ValueType1 value = *first1 * *first2;
	*result = value;
	while (++first1 != last1)
	{
		value = binary_op1(value, binary_op2(*first1, *++first2));
		*++result = value;

	}
	return ++result;
}

/**
 * The Cauchy product:
 *   c_n = \sum_{k=0}^n { a_k * b_{n-k} }
 * This product is very similar to the integral. It is not calculated only for the total size of the vector, but for
 * each element of the vector. And the element-wise multiplication iterates backwards over the second vector as in
 * convolution.
 *
 * @param 			last2 is an iterator to the LAST element of the second vector
 */
template<typename InputIterator1, typename InputIterator2, typename OutputIterator>
OutputIterator
cauchy_product(InputIterator1 first1, InputIterator1 last1, InputIterator2 last2,
		OutputIterator result) {

	typedef typename std::iterator_traits<InputIterator1>::value_type ValueType1;

	// concept requirements
	__glibcxx_function_requires(_InputIteratorConcept<InputIterator1>);
	__glibcxx_function_requires(_InputIteratorConcept<InputIterator2>);
	__glibcxx_function_requires(_OutputIteratorConcept<OutputIterator, ValueType1>);
	__glibcxx_requires_valid_range(first1,last1);

	if (first1 == last1)
		return result;

	ValueType1 value = *first1 * *--last2;
	*result = value;
	while (++first1 != last1)
	{
		value = value + *first1 * *--last2;
		*++result = value;
	}
	return ++result;
}

/**
 * Inner product where we iterate backwards over the second container.
 */
template<typename InputIterator1, typename InputIterator2, typename Tp>
inline Tp
reverse_inner_product(InputIterator1 first1, InputIterator1 last1,
		InputIterator2 last2, Tp init)
{
	// concept requirements
	__glibcxx_function_requires(_InputIteratorConcept<InputIterator1>);
	__glibcxx_function_requires(_InputIteratorConcept<InputIterator2>);
	__glibcxx_requires_valid_range(first1, last1);
	--last2;
	for (; first1 != last1; ++first1, --last2)
		init = init + (*first1 * *last2);
	return init;
}

/**
 * This function calculates the discrete convolution between two functions represented by for example vectors or sets.
 * It calculates the product of x[i] and y[shift-i]. So, with shift of 1, it multiplies x[0] with x[1], x[1] with y[0],
 * x[2] with y[-1], etc. So, a normal convolution would not work for a finite range of values (like a vector). The
 * circular_convolution however, works fine with a limited sequence. It calculates:
 *   conv_n(shift) = \sum_{k=0}^{n-1} { a_k * b_{(shift-k) % n} }
 *
 * In other words, using vector terminology. It calculates the inner product between the two vectors with the second one
 * reversed and each time shifted a bit more with "shift" (default 1) for in total N times, the results which are
 * written into an output container.
 *
 * @param			first1 beginning of container
 * @param			last1 end of container
 * @param			first2 beginning of second container
 * @param 			last2 end of second container (required for rotation)
 * @param			result begin of container for results (needs capacity last1 - first1)
 * @param			(optional) shift with which to calculate the convolution, default 1
 * @return			end of result container
 */
template<typename ForwardIterator1, typename ForwardIterator2, typename OutputIterator>
OutputIterator
circular_convolution(ForwardIterator1 first1, ForwardIterator1 last1, ForwardIterator2 first2,
		ForwardIterator2 last2, OutputIterator result, int shift = 1) {
	typedef typename std::iterator_traits<ForwardIterator1>::value_type ValueType1;

	__glibcxx_function_requires(_ForwardIteratorConcept<ForwardIterator1>);
	__glibcxx_function_requires(_ForwardIteratorConcept<ForwardIterator2>);
	__glibcxx_function_requires(_OutputIteratorConcept<OutputIterator,ValueType1>);
	__glibcxx_requires_valid_range(first1, last1);
	__glibcxx_requires_valid_range(first2, last2);

	typedef typename std::iterator_traits<ForwardIterator1>::value_type ValueType1;
	typedef typename std::iterator_traits<ForwardIterator1>::difference_type DistanceType1;

	DistanceType1 dist = std::distance(first1, last1);
	while (dist--) {
		std::rotate(first2, last2-shift, last2);
		*result++ = reverse_inner_product(first1, last1, last2, ValueType1(0));
	}
	return result;
}

/**
 * Rotating left or right.
 */
enum RotateDirection {
	RD_LEFT,
	RD_RIGHT,
};

/**
 * @brief Rotate the elements of a sequence by one.
 *
 * Rotates the elements of a sequence in either of two directions and just by one. Use the std::rotate if you want
 * something else.
 *
 * @param first   	   A forward iterator.
 * @param last    	   A forward iterator.
 * @param direction    Rotating right or left
 * @return             Nothing.
 */
template<typename InputIterator>
void rotate(InputIterator first, InputIterator last, RotateDirection direction = RD_RIGHT) {
	__glibcxx_function_requires(_InputIteratorConcept<InputIterator>);
	__glibcxx_requires_valid_range(first, last);
	if (last - first == 1) return;
	switch (direction) {
	case RD_LEFT:
		std::rotate(first, first+1, last);
		break;
	case RD_RIGHT: default:
		std::rotate(first, last-1, last);
		break;
	}
}

/**
 * @brief Rotate the elements of a sequence by N
 */
template<typename InputIterator>
void rotate_n(InputIterator first, InputIterator last, size_t N, RotateDirection direction = RD_RIGHT) {
	__glibcxx_function_requires(_InputIteratorConcept<InputIterator>);
	__glibcxx_requires_valid_range(first, last);
	if (last - first == 1) return;
	switch (direction) {
	case RD_LEFT:
		if (first+N > last) return;
		std::rotate(first, first+N, last);
		break;
	case RD_RIGHT: default:
		if (last-N < first) return;
		std::rotate(first, last-N, last);
		break;
	}
}

/**
 * Adds a vector with itself shifted one to the left or right. Shifting to the right is the default.
 */
template<typename InputIterator, typename OutputIterator>
OutputIterator
shiftadd(InputIterator first, InputIterator last, OutputIterator result, RotateDirection direction = RD_RIGHT) {
	typedef typename std::iterator_traits<InputIterator>::value_type ValueType;
	__glibcxx_function_requires(_InputIteratorConcept<InputIterator>);
	__glibcxx_function_requires(_OutputIteratorConcept<OutputIterator, ValueType>);
	__glibcxx_requires_valid_range(first, last);

	if (first == last) return result;

	if (first - last == 1) {
		*result++ = *first + *first;
		return result;
	}

	switch (direction) {
	case RD_RIGHT: default: {
		ValueType prev = *first;
		while (++first != last) {
			ValueType value = *first;
			*++result = value + prev;
			prev = value;
		}
		*++result = prev + *first;
	} break;
	case RD_LEFT: {
		ValueType prev = *first;
		*++result = *(last-1) + prev;
		while (++first != last) {
			ValueType value = *first;
			*++result = value + prev;
			prev = value;
		}
	}
	}
	return result;
}

/**
 * Summation of a window in a vector and write sum to result. The window is centered around the resulting vector
 * elements. So, "half_window" extends left as well as right. A half_window of "2" corresponds to a sliding window
 * of [-2,-1,0,1,2].
 */
template<typename InputIterator, typename OutputIterator>
OutputIterator
window_add(InputIterator first, InputIterator last, OutputIterator result, size_t half_window) {
	typedef typename std::iterator_traits<InputIterator>::value_type ValueType;
	typedef typename std::iterator_traits<InputIterator>::difference_type DistanceType;

	__glibcxx_function_requires(_InputIteratorConcept<InputIterator>);
	__glibcxx_function_requires(_OutputIteratorConcept<OutputIterator, ValueType>);
	__glibcxx_requires_valid_range(first, last);

	DistanceType dist = std::distance(first, last);
	if (!dist) return result;
	if (dist==1) return result;
	if (dist<half_window*2-1) return result;

	DistanceType index = DistanceType(0);
	for (; index != dist; ++index, ++result) {
		ValueType value = ValueType(0);
		for (int i = -half_window; i != half_window+1; ++i) {
			DistanceType d = (i + index + dist) % dist;
			value += *(first+d);
		}
		*result = value;
	}
	return result;
}


/**
 * Pushes an item upon a container and pops off the oldest value. In case the container is build up out of pointers to
 * objects, make sure you deallocate the item yourself. This can perfectly be used for a circular buffer. The
 * implementation actually physically rotates the elements so it will not be efficient, but using just integers in there
 * it might be faster than expected.
 *
 * @param first 		Iterator to the beginning of the container
 * @param last 			Iterator to the end of the container
 * @param item 			The item to be added to the container
 * @param direction 	The direction in which to push and pop (left or right)
 */
template<typename InputIterator, typename T>
void pushpop(InputIterator first, InputIterator last, T item, RotateDirection direction = RD_RIGHT) {
	__glibcxx_function_requires(_InputIteratorConcept<InputIterator>);
	__glibcxx_requires_valid_range(first, last);
	dobots::rotate(first, last, direction);
	// overwrite first item
	*first = item;
}

/**
 * The function argmin calculates unary_op(x_i) and unary_op(x_j) for all i and j in vector X and returns
 * that index that is minimum.
 * 	{x*} = argmin_x f(x)
 * The {} means that it is in principle possible to return a set (multiple minima!), which we will not do however.
 * The * signifies the local minimum of the possible x. The argmin_x will input every possible x in function f. And the
 * function takes only one argument, it is a unary_op.
 *
 * Note, that this function is actually almost the same as std::min_element() except that the latter takes a comparison
 * functor as (optional) argument and this function takes a unary operator.
 *
 * @param first              Beginning of container
 * @param last               End of container
 * @param unary_op           Function to perform
 * @param init               Minimum value (set it to first element in container if you don't want it)
 * @return                   Index to minimum value
 */
template<typename ForwardIterator, typename UnaryOperation>
ForwardIterator
argmin(ForwardIterator first, ForwardIterator last, UnaryOperation unary_op) {

	typedef typename std::iterator_traits<ForwardIterator>::value_type ValueType;

	// concept requirements
	__glibcxx_function_requires(_ForwardIteratorConcept<ForwardIterator>);
	__glibcxx_requires_valid_range(first,last);

	if (first == last)
		return first;
	ForwardIterator result = first;
	while (++first != last)
		if (unary_op(*first) < unary_op(*result))
			result = first;
	return result;
}


// end of namespace dobots

}


#endif /* DIM1ALGEBRA_HPP_ */