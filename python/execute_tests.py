from geometry import *

def geometry_tests ():
    p1 = Point(0, 0)
    p2 = Point(8, 0)

    p3 = Point(0, 8)
    p4 = Point(8, 8)

    s1 = Segment(p1, p2)
    s2 = Segment(p3, p4)

    s3 = Segment(p1, p3)
    s4 = Segment(p2, p4)

    d1 = Segment(p1, p4)
    d2 = Segment(p2, p3)


    print (f's1 = {s1}')
    print (f's2 = {s2}')

    points_collinear_test (p1, p1, p1, True)
    points_collinear_test (p1, p1, Point(5,0), True)
    points_collinear_test (p1, p2, Point(5,0), True)
    points_collinear_test (p1, p2, p3, False)

    test(points_between(p1, p2, Point(4,0)), True)

    segments_intersect_test (s1, s2, False)
    segments_intersect_test (s3, s4, False)

    segments_intersect_test (s1, s3, True)
    segments_intersect_test (s1, d1, True)
    segments_intersect_test (s1, d2, True)
    segments_intersect_test (s1, s4, True)

    segments_intersect_test (s2, s3, True)
    segments_intersect_test (s2, d1, True)
    segments_intersect_test (s2, d2, True)
    segments_intersect_test (s2, s4, True)

    segments_intersect_test (d1, d2, True)

    points_centroid_test ([p1], p1)
    points_centroid_test ([p1, p1, p1, p1], p1)
    points_centroid_test ([p1, p2, p3, p4], Point(4, 4))
    points_centroid_test ([p1, p2], Point(4, 0))
    points_centroid_test ([p1, p3], Point(0, 4))

    points_min_test ([p1, p2, p3, p4], p1)

    point_get_quadrant_test (Point( 1, 0), Point(0, 0), 1)
    point_get_quadrant_test (Point( 1, 1), Point(0, 0), 1)
    point_get_quadrant_test (Point( 0, 1), Point(0, 0), 2)
    point_get_quadrant_test (Point(-1, 1), Point(0, 0), 2)
    point_get_quadrant_test (Point(-1, 0), Point(0, 0), 3)
    point_get_quadrant_test (Point(-1,-1), Point(0, 0), 3)
    point_get_quadrant_test (Point( 0,-1), Point(0, 0), 4)
    point_get_quadrant_test (Point( 1,-1), Point(0, 0), 4)

    o = points_centroid ([p1, p2, p3, p4])
    points_sort_test ([p1, p2, p3, p4], o, [p4, p3, p1, p2])
    points_sort_test ([p1, p2, p3, p4], o, [p2, p1, p3, p4], reverse=True)
    points_sort_test ([p1, Point(8, 4), p2, p4], o, [Point(8, 4), p4, p1, p2])
    points_sort_test ([p1, Point(8, 4), p2, p4], o, [p2, p1, p4, Point(8, 4)], reverse=True)

if __name__ == "__main__":
    geometry_tests()
