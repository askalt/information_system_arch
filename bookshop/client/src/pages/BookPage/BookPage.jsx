import React, { useState, useEffect } from 'react';
import { useParams, useNavigate } from 'react-router-dom';
import { Avatar, Button, Typography, Box, Container, TextField, Rating, CircularProgress } from '@mui/material';
import { useCart } from '../CartPage/CartContext';
//import { getReviews, submitReview } from '../../mocks/reviewService';
//import { getBook } from '../../mocks/bookService';
import PriceTypography from '../../components/PriceTypography'
import Review from './BookReview';
import { useAuth } from '../../contexts/AuthContext';
import authFetch from '../../AuthFetch';
import { jwtDecode } from 'jwt-decode';

const BookPage = () => {
    const { id } = useParams();
    const [book, setBook] = useState(null);
    const [reviewText, setReviewText] = useState('');
    const [reviewRating, setRating] = useState(0);
    const [reviews, setReviews] = useState([]);
    const [loading, setLoading] = useState(true);
    const { addToCart } = useCart();
    const [user, setUser] = React.useState({});
    const { token, saveToken } = useAuth();
    const [userId, setUserId] = useState(null);
    const navigate = useNavigate();

    useEffect(() => {
        if (token) {
            console.log(token);
            setUserId(jwtDecode(token).user_id);
        }
    }, [token]);

    useEffect(() => {
        if (token) {
            const decoded = jwtDecode(token);
            fetch(`http://127.0.0.1:8000/getUser/${decoded.user_id}`)
                .then((response) => response.json())
                .then((user) => {
                    setUser(user);
                });
        }
    }, [token]);

    useEffect(() => {
        setLoading(true);
        const requests = [
            fetch(`http://127.0.0.1:8000/getBook/${id}`)
                .then((response) => response.json()),
            fetch(`http://127.0.0.1:8000/getReviews/${id}`)
                .then((response) => response.json()),
        ];
        Promise.all(requests).then(([book, reviews]) => {
            setBook(book);
            setReviews(reviews);
        }).finally(() => {
            setLoading(false);
        });
    }, [id]);

    const handleAddToCart = () => {
        if (token) {
            if (book) {
                authFetch(`http://127.0.0.1:8000/cart/${id}/add`, {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json',
                    },
                    body: JSON.stringify({ book_id: book.id }),
                }, token, saveToken)
                    .then((response) => response.json())
                    .then((cartItem) => {
                        console.log(cartItem);
                        addToCart(book);
                    })
                    .catch((error) => {
                        alert(error);
                    });
            }
        } else {
            navigate('/login');
        }
    };

    const handleSubmitReview = () => {
        if (reviewText && reviewRating > 0) {
            const review = {
                rating: reviewRating,
                text: reviewText,
                userId: userId,
            };
            authFetch(`http://127.0.0.1:8000/submitReview/${id}`, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify(review),
            }, token, saveToken)
                .then((response) => response.json())
                .then((newReview) => {
                    console.log(newReview)
                    setReviews((prevReviews) => [...prevReviews, newReview]);
                    setReviewText('');
                    setRating(0);
                })
                .catch((error) => {
                    alert(error);
                });
        } else {
            alert('Пожалуйста, оставьте отзыв и поставьте оценку');
        }
    };

    const handleDeleteReview = (review_id) => {
        authFetch(`http://127.0.0.1:8000/removeReview/${review_id}`, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
        }, token, saveToken)
            .then((response) => response.json())
            .then((review) => {
                setReviews((reviews) => reviews.filter(review => review.id != review_id));
            })
            .catch((error) => {
                alert(error);
            });
    };

    return (
        <Box sx={{ mt: 4, mb: 4 }}>
            {loading ? (
                <CircularProgress />
            ) : (
                <Container>
                    <Box sx={{ textAlign: 'center', mb: 4 }}>
                        <Typography variant="h4" gutterBottom>{book.name}</Typography>
                        <Typography variant="h6" color="textSecondary">{book.author}</Typography>
                    </Box>
                    <Box sx={{ display: 'flex', gap: 4, flexDirection: { xs: 'column', sm: 'row' } }}>
                        <Box sx={{
                            maxHeight: '500px',
                        }}>
                            <Box sx={{ overflow: 'hidden', width: '400px', height: '400px', position: 'relative' }}>
                                <img
                                    src={book.image}
                                    alt={book.name}
                                    style={{
                                        width: '100%',
                                        height: '100%',
                                        objectFit: 'contain',
                                        backgroundColor: '#f0f0f0',
                                    }}
                                />
                            </Box>
                            <PriceTypography variant="h6">{book.price}₽</PriceTypography>
                            <Button variant="contained" color="primary" sx={{ mt: 2 }} onClick={handleAddToCart}>
                                Добавить в корзину
                            </Button>
                        </Box>
                        <Box sx={{
                            flex: 1, padding: 2,
                            height: '500px',
                            overflowY: 'auto',
                            mb: 2
                        }}>
                            <Typography variant="body1" gutterBottom>Отзывы о книге:</Typography>
                            {reviews.map((review) => (
                                <Review key={review.id} review={review} onDelete={handleDeleteReview} />
                            ))}
                        </Box>
                    </Box>

                    {token ? (
                        <Box sx={{ mt: 2 }}>
                            <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', mb: 2 }}>
                                <Box sx={{ display: 'flex', alignItems: 'center' }}>
                                    <Avatar alt={user.name} src={user.image} sx={{ mr: 2 }} />
                                    <Typography variant="body1" fontWeight="bold">{user.name}</Typography>
                                </Box>
                                <Rating value={reviewRating} onChange={(event, newValue) => setRating(newValue)} />
                            </Box>

                            <TextField
                                label="Ваш отзыв"
                                fullWidth
                                multiline
                                rows={4}
                                value={reviewText}
                                onChange={(e) => setReviewText(e.target.value)}
                                sx={{ mb: 2 }}
                            />

                            <Button
                                variant="contained"
                                color="primary"
                                onClick={handleSubmitReview}
                                disabled={!reviewText || reviewRating === 0}
                            >
                                Оставить отзыв
                            </Button>
                        </Box>) : (
                        <Box> Авторизуйтесь, чтобы оставить отзыв </Box>
                    )}
                </Container>
            )}
        </Box >
    );
};

export default BookPage;