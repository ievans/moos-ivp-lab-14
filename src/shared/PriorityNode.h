#ifndef PRIORITYNODE_HEADER
#define PRIORITYNODE_HEADER

class PriorityNode {
    double weight;
    Point2D point; 

public:
    PriorityNode(double weight, Point2D point) {
	this->weight = weight;
	this->point = point;
    };

    Point2D getPoint() {
	return point;
    }

    double getWeight() {
	return weight;
    }

    const bool operator <(const PriorityNode& pn) const {
	return (weight < pn.weight);
    };

};

#endif
