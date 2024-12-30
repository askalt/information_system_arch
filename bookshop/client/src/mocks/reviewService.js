const MOCK_REVIEWS = [
    { id: 1, rating: 5, text: 'Отличная книга! Рекомендую всем.', userId: 1 },
    { id: 2, rating: 4, text: 'Хорошая книга, но местами скучная.', userId: 2 },
];

export const getReviews = (bookId) => {
    return new Promise((resolve) => {
        setTimeout(() => {
            resolve(MOCK_REVIEWS);
        }, 100);
    });
};

export const submitReview = (bookId, review) => {
    return new Promise((resolve, reject) => {
        setTimeout(() => {
            if (review.text && review.rating) {
                review.id = MOCK_REVIEWS.length + 1;
                MOCK_REVIEWS.push(review);
                resolve(review);
            } else {
                reject('Отзыв не может быть пустым.');
            }
        }, 100);
    });
};