#include "focusedpath.h"

#include "focusedpathprotocol.h"
#include "focusededge.h"
#include "zintpoint.h"
#include "zjsonparser.h"

#include "dvid/zdvidannotation.h"
#include "dvid/zdvidreader.h"

/*
 * this class is a lightweight container for a list of edges = a path
 * between bodies; it does minimal loading and no saving itself
 *
 * see DVID data format here: https://wiki.janelia.org/wiki/display/flyem/NeuTu+focused+protocol+DVID+data+formats
 *
 */
FocusedPath::FocusedPath() {
    // I hate C++
}

FocusedPath::FocusedPath(ZDvidAnnotation annotation)
{
    m_firstPoint = annotation.getPosition();

    // last point; the relation should only have one point
    m_lastPoint = ZJsonParser::toIntPoint(((ZJsonObject) annotation.getRelationJson().value(0))["To"]);

    // probability
    m_probability = atof(annotation.getProperty<std::string>(FocusedPathProtocol::PROPERTY_PROBABILITY).c_str());

    // edge points
    ZJsonArray edgeList;
    edgeList.decode(annotation.getProperty<std::string>(FocusedPathProtocol::PROPERTY_PATH));
    for (size_t i=0; i<edgeList.size(); i++) {
        m_edgePoints.append(ZJsonParser::toIntPoint(edgeList.at(i)));
    }

    // we do not load the actual edges right away

}

bool FocusedPath::operator ==(const FocusedPath& other) const {
    // needed to get QList to behave
    return m_firstPoint == other.getFirstPoint() && m_lastPoint == other.getLastPoint();
}

ZIntPoint FocusedPath::getFirstPoint() const {
    return m_firstPoint;
}

ZIntPoint FocusedPath::getLastPoint() const {
    return m_lastPoint;
}

void FocusedPath::setProbability(double probability) {
    m_probability = probability;
}

double FocusedPath::getProbability() {
    return m_probability;
}

FocusedEdge FocusedPath::getEdge(ZIntPoint point) {
    return m_edgeMap[point];
}

FocusedEdge FocusedPath::getEdge(int i) {
    return getEdge(m_edgePoints[i]);
}

int FocusedPath::getNumEdges() {
    return m_edgePoints.size();
}

void FocusedPath::loadEdges(ZDvidReader& reader, std::string instance) {
    // unfortunately, no way to bulk load annotations by point right now

    m_edgeMap.clear();

    std::vector<ZIntPoint> points;

    foreach(ZIntPoint point, m_edgePoints) {

        ZJsonObject jsonEdge = reader.readAnnotationJson(instance, point);
        FocusedEdge edge(jsonEdge);

        m_edgeMap[edge.getFirstPoint()] = edge;
        m_edgeMap[edge.getLastPoint()] = edge;

        points.push_back(edge.getFirstPoint());
        points.push_back(edge.getLastPoint());
    }

    // load all the body IDs at the points
    m_bodyIDs.clear();
    std::vector<uint64_t> bodyIDs = reader.readBodyIdAt(points);
    for (size_t i=0; i<points.size(); i++) {
        m_bodyIDs[points[i]] = bodyIDs[i];
    }

    // one more loop...fill in body IDs in edges; can't do earlier
    //  because we need 2 body IDs for each edge
    foreach (FocusedEdge edge, m_edgeMap.values()) {
        edge.setFirstBodyID(m_bodyIDs[edge.getFirstPoint()]);
        edge.setLastBodyID(m_bodyIDs[edge.getLastPoint()]);
    }

}

bool FocusedPath::hasEdges() {
    return getNumEdges() > 0;
}

bool FocusedPath::isConnected() {

    ZIntPoint prevPoint = m_edgePoints.first();
    foreach(ZIntPoint point, m_edgePoints) {
        // connected to previous edge (could have been split)?
        if (m_bodyIDs[point] != m_bodyIDs[prevPoint]) {
            return false;
        }

        // connected across edge (could have been done some other time)?
        FocusedEdge edge = getEdge(point);
        if (m_bodyIDs[point] != m_bodyIDs[edge.getOtherPoint(point)]) {
            return false;
        }

        prevPoint = edge.getOtherPoint(point);
    }
    return true;
}

std::string FocusedPath::getConnectionTextIcon() {
    if (isConnected()) {
        return FocusedEdge::GLYPH_CONNECTED;
    } else if (getNumUnexaminedEdges() > 0) {
        return FocusedEdge::GLYPH_UNKNOWN;
    } else {
        return FocusedEdge::GLYPH_UNCONNECTED;
    }
}

int FocusedPath::getNumUnexaminedEdges() {
    int total = 0;
    foreach (ZIntPoint point, m_edgePoints) {
        if (!m_edgeMap[point].isExamined()) {
            total += 1;
        }
    }
    return total;
}

int FocusedPath::getFirstUnexaminedEdgeIndex() {
    for (int i=0; i<m_edgePoints.size(); i++) {
        if (!getEdge(i).isExamined()) {
            return i;
        }
    }
    return -1;
}













